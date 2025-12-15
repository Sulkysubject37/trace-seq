library(rprojroot)

# Function to get the project root path
get_project_root <- function() {
  return(rprojroot::find_root(rprojroot::has_file("README.md")))
}

project_root <- get_project_root()

source(file.path(project_root, "r", "trace_load.R"))
source(file.path(project_root, "r", "trace_ontology.R"))
source(file.path(project_root, "r", "trace_annotate.R"))
source(file.path(project_root, "r", "trace_report.R"))
source(file.path(project_root, "r", "trace_validate.R"))
