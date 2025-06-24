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
  # The release version must be published manually using the `publishPlugins` Gradle task
  mkdir -p tools/slice-tools/build/libs/
  cp -pf "${STAGING_DIR}"/java-packages/tools/slice-tools/build/libs/*.jar tools/slice-tools/build/libs/
  cp -pf "${STAGING_DIR}"/java-packages/tools/slice-tools/build/libs/*.asc tools/slice-tools/build/libs/

  base_name="slice-tools-${ice_version}"
  artifact_dir="tools/slice-tools/build/libs"

  jar="${artifact_dir}/${base_name}.jar"
  pom_file="${artifact_dir}/${base_name}.pom"

  echo "Publishing $base_name"

  mvn deploy:deploy-file \
    -Dfile="${jar}" \
    -DpomFile="${pom_file}" \
    -Dfiles="${pom_file}.asc,${jar}.asc" \
    -Dtypes=pom.asc,jar.asc \
    -Dclassifiers=,jar \
    -Durl="${SOURCE_URL}" \
    -DrepositoryId="${REPO_ID}" || { echo "Failed to publish $base_name"; exit 1; }
fi
