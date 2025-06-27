set -euo pipefail

case "$CHANNEL" in
  "3.8")
    REPO_ID=ossrh
    SOURCE_URL="https://s01.oss.sonatype.org/service/local/staging/deploy/maven2/"
    MAVEN_USERNAME="${MAVEN_CENTRAL_USERNAME}"
    MAVEN_PASSWORD="${MAVEN_CENTRAL_API_KEY}"
    ;;
  "nightly")
    REPO_ID=zeroc-nightly
    SOURCE_URL="https://download.zeroc.com/nexus/repository/maven-nightly/"
    MAVEN_USERNAME="nightly"
    MAVEN_PASSWORD="${NEXUS_NIGHTLY_PASSWORD}"
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

mkdir -p lib
cp -pf "${STAGING_DIR}"/java-packages/lib/*.jar lib
cp -pf "${STAGING_DIR}"/java-packages/lib/*.pom lib
cp -pf "${STAGING_DIR}"/java-packages/lib/*.asc lib

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

  mvn deploy:deploy-file \
    -Dfile="${jar}" \
    -Djavadoc="${javadoc_jar}" \
    -Dsources="${sources_jar}" \
    -DpomFile="${pom_file}" \
    -Dfiles="${pom_file}.asc,${jar}.asc,${javadoc_jar}.asc,${sources_jar}.asc" \
    -Dtypes=pom.asc,jar.asc,javadoc.jar.asc,sources.jar.asc \
    -Dclassifiers=,jar,javadoc,sources \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish $base_name"; exit 1; }
done

if [ "$CHANNEL" = "nightly" ]; then
  echo "Publishing Slice Tools plugin"

  plugin_jar="${STAGING_DIR}/com/zeroc/ice/slice-tools/${ice_version}/slice-tools-${ice_version}.jar"
  plugin_pom="${STAGING_DIR}/com/zeroc/ice/slice-tools/${ice_version}/slice-tools-${ice_version}.pom"

  mvn deploy:deploy-file \
    -Dfile="${plugin_jar}" \
    -DpomFile="${plugin_pom}" \
    -Dfiles="${plugin_jar}.asc,${plugin_pom}.asc" \
    -Dtypes=jar.asc,pom.asc \
    -Dclassifiers=, \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish plugin"; exit 1; }

  plugin_marker_pom="${STAGING_DIR}/com/zeroc/ice/slice-tools/com.zeroc.ice.slice-tools.gradle.plugin/${ice_version}/com.zeroc.ice.slice-tools.gradle.plugin-${ice_version}.pom"
  plugin_marker_asc="${plugin_marker_pom}.asc"

  echo "Publishing plugin marker POM"

  mvn deploy:deploy-file \
    -Dfile="${plugin_marker_pom}" \
    -Dpackaging=pom \
    -DgroupId="com.zeroc.ice.slice-tools" \
    -DartifactId="com.zeroc.ice.slice-tools.gradle.plugin" \
    -Dversion="${ice_version}" \
    -Dfiles="${plugin_marker_asc}" \
    -Dtypes=pom.asc \
    -Dclassifiers= \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish plugin marker"; exit 1; }
fi
