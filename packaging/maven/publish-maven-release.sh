set -xeuo pipefail

case "$QUALITY" in
  "stable")
    REPO_ID=ossrh
    SOURCE_URL="https://ossrh-staging-api.central.sonatype.com/service/local/staging/deploy/maven2/"
    ;;
  *)
    REPO_ID=maven-${CHANNEL}-${QUALITY}
    SOURCE_URL="https://download.zeroc.com/nexus/repository/maven-${CHANNEL}-${QUALITY}/"
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
mkdir -p lib/java lib/java-compat

# Copy staged artifacts (3.7 layout)
cp -pf "${STAGING_DIR}/java-packages/java/lib/"*.jar "${STAGING_DIR}/java-packages/java/lib/"*.pom lib/java
cp -pf "${STAGING_DIR}/java-packages/java-compat/lib/"*.jar "${STAGING_DIR}/java-packages/java-compat/lib/"*.pom lib/java-compat

# Determine Ice version

ice_version=$(basename "$(ls lib/java/ice-*-sources.jar)" | sed -E 's/^ice-(.+)-sources\.jar$/\1/')
components=(ice glacier2 icebt icebox icediscovery icelocatordiscovery icegrid icestorm icepatch2 icessl)

publish_component() {
  local artifact_dir="$1"   # lib/java or lib/java-compat
  local artifact_id="$2"    # ice / ice-compat / etc
  local base_name="${artifact_id}-${ice_version}"

  local jar="${artifact_dir}/${base_name}.jar"
  local javadoc_jar="${artifact_dir}/${base_name}-javadoc.jar"
  local sources_jar="${artifact_dir}/${base_name}-sources.jar"
  local pom_file="${artifact_dir}/${base_name}.pom"

  # Skip if this artifact doesn't exist in this dir (keeps script resilient)
  [[ -f "${jar}" ]] || return 0

  # Basic sanity checks (fail fast with a clear message)
  for f in "${javadoc_jar}" "${sources_jar}" "${pom_file}"; do
    if [[ ! -f "${f}" ]]; then
      echo "ERROR: Missing required file for ${base_name}: ${f}"
      exit 1
    fi
  done

  echo "Publishing ${base_name} (from ${artifact_dir})"

  mvn org.apache.maven.plugins:maven-gpg-plugin:3.2.4:sign-and-deploy-file \
    -Dgpg.keyname="${GPG_KEY_ID}" \
    -Dfile="${jar}" \
    -Djavadoc="${javadoc_jar}" \
    -Dsources="${sources_jar}" \
    -DpomFile="${pom_file}" \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish ${base_name}"; exit 1; }
}

# Publish normal artifacts from java/lib
for component in "${components[@]}"; do
  publish_component "lib/java" "${component}"
done

# Publish compat artifacts from java-compat/lib (artifactId is <component>-compat)
for component in "${components[@]}"; do
  publish_component "lib/java-compat" "${component}-compat"
done

if [ "$QUALITY" = "stable" ]; then
  # Tell maven central to validate the deployed artifacts, the deployment needs to be manually published
  # from the maven central web site after it has been validated.
  curl -sS -X POST \
  -H "Authorization: Bearer ${MAVEN_CENTRAL_BEARER_AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  "https://ossrh-staging-api.central.sonatype.com/manual/upload/defaultRepository/com.zeroc" \
  -d '{"publishing_type":"user_managed"}'
fi
