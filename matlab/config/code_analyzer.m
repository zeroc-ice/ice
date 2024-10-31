function code_analyzer()
    result = codeIssues(["../lib/+Ice", "../lib/+IceInternal", "../lib/generated"], CodeAnalyzerConfiguration="analyzer.json");
    export(result, "result.json", FileFormat="json");
    disp(result);
    if height(result.Issues) > 0
        exit(1);
    else
        exit(0);
    end
end
