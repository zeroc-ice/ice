import { defineConfig } from "eslint/config";
import globals from "globals";
import js from "@eslint/js";

export default defineConfig([
    {
        ignores: [
            "coverage-report/**",
            "dist/**",
            "test/**", // TODO fix linting issues in test files
        ],
    },
    {
        files: ["**/*.{js,mjs,cjs}"],
        languageOptions: {
            globals: { ...globals.browser, ...globals.node, ...globals.worker },
        },
        plugins: { js },
        extends: ["js/recommended"],
        rules: {
            "no-unused-labels": "off",
        },
        linterOptions: {
            reportUnusedDisableDirectives: "off",
        },
    },
]);
