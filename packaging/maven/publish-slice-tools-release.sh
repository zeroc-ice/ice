set -xeuo pipefail

plugin_staging_dir="${STAGING_DIR}/slice-tools-packages/com/zeroc/slice-tools"
ice_version=$(basename "$(ls -d "${plugin_staging_dir}"/[0-9]*/)")

if [ "$QUALITY" = "stable" ]; then
  echo "Publishing Slice Tools plugin to the Gradle Plugin Portal"

  : "${GRADLE_PUBLISH_KEY:?GRADLE_PUBLISH_KEY is required}"
  : "${GRADLE_PUBLISH_SECRET:?GRADLE_PUBLISH_SECRET is required}"

  plugin_jar="${plugin_staging_dir}/${ice_version}/slice-tools-${ice_version}.jar"

  if [ ! -f "${plugin_jar}" ]; then
    echo "Slice Tools plugin jar not found at ${plugin_jar}"
    exit 1
  fi

  plugin_project_dir="java/tools/slice-tools"
  plugin_resources_dir="${plugin_project_dir}/src/main/resources"

  rm -rf "${plugin_resources_dir}/resources"
  mkdir -p "${plugin_resources_dir}"
  unzip -qo "${plugin_jar}" "resources/*" -d "${plugin_resources_dir}"

  pushd "${plugin_project_dir}" > /dev/null

  ../../gradlew publishPlugins \
    -Pgradle.publish.key="${GRADLE_PUBLISH_KEY}" \
    -Pgradle.publish.secret="${GRADLE_PUBLISH_SECRET}"

  popd > /dev/null
else
  echo "Publishing Slice Tools plugin"

  source "$(dirname "${BASH_SOURCE[0]}")/../common/setup-maven.sh"
  setup_maven

  mkdir -p plugin

  rm -f "${plugin_staging_dir}/${ice_version}/"*-javadoc.jar
  rm -f "${plugin_staging_dir}/${ice_version}/"*-sources.jar
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
