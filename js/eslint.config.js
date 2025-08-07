import eslint from "@eslint/js";
import tseslint from "typescript-eslint";
import globals from "globals";

export default tseslint.config(
    eslint.configs.recommended,
    tseslint.configs.recommended,
    {
        ignores: ["test/**/*.js", "dist/**", "coverage-report/**", "docs/**"],
    },
    {
        linterOptions: {
            reportUnusedDisableDirectives: "off",
        },
    },
    {
        // Override for specific paths
        files: ["src/**/*.d.ts"],
        rules: {
            "@typescript-eslint/triple-slash-reference": "off",
        },
    },
    {
        files: ["**/*.d.ts"],
        rules: {
            "@typescript-eslint/no-explicit-any": "off",
        }
    },
    {
        rules: {
            "@typescript-eslint/no-unused-vars": [
                "error",
                {
                    args: "all",
                    argsIgnorePattern: "^_",
                    caughtErrors: "all",
                    caughtErrorsIgnorePattern: "^_",
                    destructuredArrayIgnorePattern: "^_",
                    varsIgnorePattern: "^_",
                    ignoreRestSiblings: true,
                },
            ],
        },
    },
    {
        files: ["src/**/*.js", "test/**/*.js", "gulpfile.js", "dts-bundle.js", "scripts/*.js"],
        languageOptions: {
            globals: { ...globals.browser, ...globals.node, ...globals.worker },
        },
    },
    {
        files: ["src/**/*.js"],
        extends: [tseslint.configs.disableTypeChecked],
    },
);
