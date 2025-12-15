library(knitr)

#' Generate a summary report for a lineage
#'
#' @param lineage A list of trace nodes representing the lineage.
#' @param output_file The path to the output file for the report.
#' @return Invisibly returns the path to the generated report.
generate_lineage_report <- function(lineage, output_file = "trace_report.md") {
  
  # Create a template for the report
  report_template <- "
# TRACE-SEQ Lineage Report

## Lineage Summary

This report summarizes the semantic provenance of the data.

{{#lineage}}
### Step {{index}}: {{operation.class}} - {{operation.method}}

*   **Trace ID:** {{trace_id}}
*   **Timestamp:** {{timestamp}}
*   **Data Class:** {{output.data_class}}
*   **Assumptions:** 
    {{#assumptions}}
    *   {{.}}
    {{/assumptions}}
---
{{/lineage}}
"
  
  # Add an index to each node for the report
  for (i in 1:length(lineage)) {
    lineage[[i]]$index <- i
  }
  
  # Render the report using whisker.render from knitr
  report_content <- whisker::whisker.render(report_template, list(lineage = lineage))
  
  # Write the report to a file
  writeLines(report_content, output_file)
  
  return(invisible(output_file))
}
