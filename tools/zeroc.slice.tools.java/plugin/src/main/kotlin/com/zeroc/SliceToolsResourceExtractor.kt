import org.gradle.api.Project
import java.io.File
import java.nio.file.Files
import java.nio.file.StandardCopyOption

/**
 * Extracts the Slice compiler (slice2java) and Slice files from the plugin resources to the Gradle user home directory.
 */
object SliceToolsResourceExtractor {
    private const val pluginName = "slice-tools"

    /**
     * Extracts the Slice compiler (slice2java) from the plugin resources to the Gradle user home directory.
     *
     * @param project The Gradle project
     * @return The path to the directory containing the Slice compiler, or null if the extraction failed or the
     * compiler is not present in the resources.
     */
    fun extractSliceCompiler(project: Project): String? {
        val pluginVersion = getPluginVersion(project)

        val os = detectOS()
        val arch = detectArch()
        val toolName = if (os == "windows") "slice2java.exe" else "slice2java"
        val resourcePath = "/resources/slice2java-$os-$arch/$toolName"

        val gradleUserDir = File(project.gradle.gradleUserHomeDir, "native/$pluginName-$pluginVersion/bin")
        val slice2JavaFile = File(gradleUserDir, toolName)

        if (!slice2JavaFile.exists()) {
            val resourceStream = this::class.java.getResourceAsStream(resourcePath)
            if (resourceStream != null) {
                project.logger.lifecycle("Extracting $toolName to ${gradleUserDir.absolutePath}")
                gradleUserDir.mkdirs()
                Files.copy(resourceStream, slice2JavaFile.toPath(), StandardCopyOption.REPLACE_EXISTING)
                slice2JavaFile.setExecutable(true)
            } else {
                project.logger.warn("$toolName is not present in resources ($resourcePath)")
                return null
            }
        }

        return gradleUserDir.absolutePath
    }

    /**
     * Extracts the Slice files from the plugin resources to the Gradle user home directory.
     *
     * @param project The Gradle project
     * @return The path to the directory containing the Slice files, or null if the extraction failed or the
     * Slice files are not present in the resources.
     */
    fun extractSliceFiles(project: Project): String? {
        val pluginVersion = getPluginVersion(project)

        val sliceSourcePath = "/slice/"
        val sliceTargetDir = File(project.gradle.gradleUserHomeDir, "native/$pluginName-$pluginVersion/slice")

        if (!sliceTargetDir.exists()) {
            project.logger.lifecycle("Extracting Slice files to ${sliceTargetDir.absolutePath}")
            sliceTargetDir.mkdirs()

            val resourceURL = this::class.java.getResource(sliceSourcePath)
            if (resourceURL == null) {
                project.logger.warn("Slice directory is missing from the resources ($sliceSourcePath)")
                return null
            }

            val resourceDir = File(resourceURL.toURI())
            copyDirectory(resourceDir, sliceTargetDir)
        }

        return sliceTargetDir.absolutePath
    }

    private fun getPluginVersion(project: Project): String {
        return project.findProperty("pluginVersion") as String?
            ?: "3.8.0a"
    }

    private fun copyDirectory(source: File, destination: File) {
        source.walk().forEach { file ->
            val targetFile = File(destination, file.relativeTo(source).path)
            if (file.isDirectory) {
                targetFile.mkdirs()
            } else {
                file.inputStream().use { input ->
                    targetFile.outputStream().use { output -> input.copyTo(output) }
                }
            }
        }
    }

    private fun detectOS(): String {
        val os = System.getProperty("os.name").lowercase()
        return when {
            os.contains("win") -> "windows"
            os.contains("mac") -> "macos"
            os.contains("nix") || os.contains("nux") -> "linux"
            else -> os
        }
    }

    private fun detectArch(): String {
        val arch = System.getProperty("os.arch").lowercase()
        return when {
            arch.contains("64") || arch.contains("x86_64") -> "x64"
            arch.contains("arm") || arch.contains("aarch64") -> "arm64"
            else -> arch
        }
    }
}
