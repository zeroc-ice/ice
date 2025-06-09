/*
 * Gradle build file for the Slice Tools plugin.
 */

val runningInCi = providers.environmentVariable("CI").isPresent

plugins {
    // Apply the Java Gradle Plugin Development plugin
    `java-gradle-plugin`
    `maven-publish`

    // Apply the Kotlin JVM plugin
     id("org.jetbrains.kotlin.jvm") version "2.1.0"

    // Code format parsing with Checkstyle
    id("checkstyle")

    // Automated code formatting based on Checkstyle with OpenRewrite
    id("org.openrewrite.rewrite") version "7.7.0"
}

repositories {
    google() // Required for AGP (Android Gradle Plugin)
    mavenCentral()
}

dependencies {
    compileOnly("com.android.tools.build:gradle:8.1.0")
    testImplementation("org.jetbrains.kotlin:kotlin-test-junit5")
    testRuntimeOnly("org.junit.platform:junit-platform-launcher")
}

// Plugin version management
val sliceToolsVersion: String by project
version = sliceToolsVersion
group = "com.zeroc.ice"

gradlePlugin {
    plugins {
        create("sliceTools") {
            id = "com.zeroc.ice.slice-tools"
            implementationClass = "com.zeroc.ice.slice.tools.SliceToolsPlugin"
        }
    }
}

tasks.withType<Jar> {
    manifest {
        attributes(
            "Implementation-Version" to sliceToolsVersion
        )
    }
}

// Publishing
publishing {
    repositories {
        val mavenRepoUrl = findProperty("mavenRepository") as String? ?: System.getenv("MAVEN_REPOSITORY")

        if (!mavenRepoUrl.isNullOrBlank()) {
            maven {
                url = uri(mavenRepoUrl)
                credentials {
                    username = findProperty("mavenUsername") as String? ?: System.getenv("MAVEN_USERNAME").orEmpty()
                    password = findProperty("mavenPassword") as String? ?: System.getenv("MAVEN_PASSWORD").orEmpty()
                }
            }
        }
    }
}

// Configure the functional test source set
val functionalTestSourceSet = sourceSets.create("functionalTest")

configurations["functionalTestImplementation"].extendsFrom(configurations["testImplementation"])
configurations["functionalTestRuntimeOnly"].extendsFrom(configurations["testRuntimeOnly"])

// Register functional test task
val functionalTest by tasks.registering(Test::class) {
    testClassesDirs = functionalTestSourceSet.output.classesDirs
    classpath = functionalTestSourceSet.runtimeClasspath
    useJUnitPlatform()
}

gradlePlugin.testSourceSets.add(functionalTestSourceSet)

// Ensure functional tests run as part of the `check` task
tasks.named<Task>("check") {
    dependsOn(functionalTest)
}

// Configure unit tests to use JUnit Jupiter
tasks.named<Test>("test") {
    useJUnitPlatform()
}

checkstyle {
    toolVersion = "10.21.4"

    // If we're running in CI, we want the build to fail if any warnings are emitted.
    // This doesn't affect what checkstyle emits, just whether it 'fails' or 'succeeds' from Gradle's perspective.
    if (runningInCi) {
        maxWarnings = 0
    }
}

rewrite {
    activeRecipe("com.zeroc.IceRewriteRecipes")
    activeStyle("com.zeroc.IceRewriteStyle")
}

// Set whether or not 'rewriteDryRun' should be considered 'failed' when it would make changes.
tasks.named("rewriteDryRun").configure {
    rewrite.failOnDryRunResults = runningInCi
}
