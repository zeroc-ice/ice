set -xeuo pipefail

source "$(dirname "${BASH_SOURCE[0]}")/../common/setup-maven.sh"
setup_maven

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

if [ "$QUALITY" = "stable" ]; then
  # Tell maven central to validate the deployed artifacts, the deployment needs to be manually published
  # from the maven central web site after it has been validated.
  curl -sS -X POST \
  -H "Authorization: Bearer ${MAVEN_CENTRAL_BEARER_AUTH_TOKEN}" \
  -H "Content-Type: application/json" \
  "https://ossrh-staging-api.central.sonatype.com/manual/upload/defaultRepository/com.zeroc" \
  -d '{"publishing_type":"user_managed"}'
fi
