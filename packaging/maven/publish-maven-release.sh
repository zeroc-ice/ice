set -xeuo pipefail

case "$CHANNEL" in
  "3.8")
    REPO_ID=ossrh
    SOURCE_URL="https://ossrh-staging-api.central.sonatype.com/service/local/staging/deploy/maven2/"
    ;;
  "nightly")
    REPO_ID=maven-snapshots
    SOURCE_URL="https://central.sonatype.com/repository/maven-snapshots/"
    ;;
  *)
    echo "Unsupported channel: $CHANNEL"
    exit 1
    ;;
esac

# Generate Maven settings.xml
mkdir -p ~/.m2
cat > ~/.m2/settings.xml <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<settings xmlns="http://maven.apache.org/SETTINGS/1.0.0"
          xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
          xsi:schemaLocation="http://maven.apache.org/SETTINGS/1.0.0 https://maven.apache.org/xsd/settings-1.0.0.xsd">
  <servers>
    <server>
      <id>${REPO_ID}</id>
      <username>${MAVEN_USERNAME}</username>
      <password>${MAVEN_PASSWORD}</password>
    </server>
  </servers>
</settings>
EOF

# Import the signing GPG key.
echo "$GPG_KEY" | gpg --batch --import

# Copy the JAR and POM files
mkdir -p lib
cp -pf "${STAGING_DIR}"/java-packages/*.jar lib
cp -pf "${STAGING_DIR}"/java-packages/*.pom lib

ice_version=$(basename "$(ls lib/ice-*-sources.jar)" | sed -E 's/^ice-(.+)-sources\.jar$/\1/')
components=(ice glacier2 icebt icebox icediscovery icelocatordiscovery icegrid icestorm)

for component in "${components[@]}"; do
  base_name="${component}-${ice_version}"
  artifact_dir="lib"

  jar="${artifact_dir}/${base_name}.jar"
  javadoc_jar="${artifact_dir}/${base_name}-javadoc.jar"
  sources_jar="${artifact_dir}/${base_name}-sources.jar"
  pom_file="${artifact_dir}/${base_name}.pom"

  echo "Publishing $base_name"

  mvn org.apache.maven.plugins:maven-gpg-plugin:3.2.4:sign-and-deploy-file \
    -Dgpg.keyname="${GPG_KEY_ID}" \
    -Dfile="${jar}" \
    -Djavadoc="${javadoc_jar}" \
    -Dsources="${sources_jar}" \
    -DpomFile="${pom_file}" \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish $base_name"; exit 1; }
done

if [ "$CHANNEL" = "3.8" ]; then
  # Tell maven central to validate the deployed artifacts, the deployment needs to be manually published
  # from the maven central web site after it has been validated.
  curl -sS -X POST \
  -H "Authorization: Bearer ${MAVEN_CENTRAL_BEARER_AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  "https://ossrh-staging-api.central.sonatype.com/manual/upload/defaultRepository/com.zeroc" \
  -d '{"publishing_type":"user_managed"}'
fi

if [ "$CHANNEL" = "nightly" ]; then
  echo "Publishing Slice Tools plugin"

  mkdir -p plugin

  plugin_staging_dir="${STAGING_DIR}/slice-tools-packages/com/zeroc/slice-tools"

  cp "${plugin_staging_dir}/${ice_version}/"*.jar "plugin/slice-tools-${ice_version}.jar"
  cp "${plugin_staging_dir}/${ice_version}/"*.pom "plugin/slice-tools-${ice_version}.pom"

  plugin_jar="plugin/slice-tools-${ice_version}.jar"
  plugin_pom="plugin/slice-tools-${ice_version}.pom"

  mvn org.apache.maven.plugins:maven-gpg-plugin:3.2.4:sign-and-deploy-file \
    -Dgpg.keyname="${GPG_KEY_ID}" \
    -Dfile="${plugin_jar}" \
    -DpomFile="${plugin_pom}" \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish plugin"; exit 1; }

  cp "${plugin_staging_dir}/com.zeroc.slice-tools.gradle.plugin/${ice_version}/"*.pom \
    "plugin/com.zeroc.slice-tools.gradle.plugin-${ice_version}.pom"

  plugin_marker_pom="plugin/com.zeroc.slice-tools.gradle.plugin-${ice_version}.pom"

  echo "Publishing plugin marker POM"

  mvn org.apache.maven.plugins:maven-gpg-plugin:3.2.4:sign-and-deploy-file \
    -Dgpg.keyname="${GPG_KEY_ID}" \
    -Dfile="${plugin_marker_pom}" \
    -Dpackaging=pom \
    -DgroupId="com.zeroc.slice-tools" \
    -DartifactId="com.zeroc.slice-tools.gradle.plugin" \
    -Dversion="${ice_version}" \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish plugin marker"; exit 1; }
fi
