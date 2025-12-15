library(digest)
library(uuid)
library(jsonlite)
library(yaml)

#' Create a new trace node
#'
#' @param parent_id The ID of the parent trace node.
#' @param data_class The data class of the input file.
#' @param operation_class The operation class.
#' @param operation_method The operation method.
#' @param assumptions A list of assumptions.
#' @return A list representing the new trace node.
create_trace_node_r <- function(parent_id, data_class, operation_class, operation_method, assumptions) {
  node <- list(
    trace_id = UUIDgenerate(),
    parent = parent_id,
    timestamp = format(Sys.time(), "%Y-%m-%dT%H:%M:%SZ"),
    data_class = data_class,
    operation = list(
      class = operation_class,
      method = operation_method
    ),
    assumptions = assumptions,
    input = list(
      shape = "unknown",
      checksum = ""
    ),
    output = list(
      data_class = "unknown",
      unit = "",
      checksum = ""
    ),
    environment = list(
      language = "R",
      tool = "traceseq-r",
      version = "0.1.0"
    ),
    ontology_version = "1.0"
  )
  return(node)
}

#' Annotate a file with a new trace
#'
#' @param filepath The path to the file to annotate.
#' @param operation_class The operation class.
#' @param operation_method The operation method.
#' @param assumptions A list of assumptions.
#' @param parent_id The ID of the parent trace node.
#' @param project_root The path to the project root.
#' @return The ID of the new trace node.
annotate_r <- function(filepath, operation_class, operation_method, assumptions = list(), parent_id = "null", project_root) {
  STORE_PATH <- file.path(project_root, ".traceseq")
  
  ontologies <- list(
    operations = load_operation_ontology(project_root),
    assumptions = load_assumption_ontology(project_root)
  )
  
  validate_operation(operation_class, ontologies)
  for (assumption in assumptions) {
    validate_assumption(assumption, ontologies)
  }
  
  input_checksum <- digest(filepath, algo = "sha256", file = TRUE)
  
  node <- create_trace_node_r(
    parent_id,
    "unknown", # data_class
    operation_class,
    operation_method,
    assumptions
  )
  
  node$input$checksum <- input_checksum
  node$output$data_class <- "unknown" # Placeholder
  node$output$checksum <- input_checksum # Placeholder
  
  # Save the node
  node_path <- file.path(STORE_PATH, "nodes", paste0(node$trace_id, ".yaml"))
  # Create .traceseq/nodes directory if it doesn't exist
  dir.create(file.path(STORE_PATH, "nodes"), recursive = TRUE, showWarnings = FALSE)
  write_yaml(node, node_path)
  
  # Update the index
  index <- load_trace_index(project_root)
  index[[input_checksum]] <- node$trace_id
  index[[node$output$checksum]] <- node$trace_id
  
  # Create .traceseq directory if it doesn't exist
  dir.create(STORE_PATH, recursive = TRUE, showWarnings = FALSE)
  write_json(index, file.path(STORE_PATH, "index.json"), auto_unbox = TRUE)
  
  return(node$trace_id)
}
