library(yaml)
library(jsonlite)
library(digest)
library(rprojroot)

#' Load a trace node from a YAML file
#'
#' @param trace_id The UUID of the trace node to load.
#' @param project_root The path to the project root.
#' @return A list representing the trace node.
load_trace_node <- function(trace_id, project_root) {
  STORE_PATH <- file.path(project_root, ".traceseq")
  file_path <- file.path(STORE_PATH, "nodes", paste0(trace_id, ".yaml"))
  if (!file.exists(file_path)) {
    stop("Trace node not found.")
  }
  return(read_yaml(file_path))
}

#' Load the trace index
#'
#' @param project_root The path to the project root.
#' @return A list representing the trace index.
load_trace_index <- function(project_root) {
  STORE_PATH <- file.path(project_root, ".traceseq")
  file_path <- file.path(STORE_PATH, "index.json")
  if (!file.exists(file_path)) {
    # If index.json does not exist, return an empty list
    return(list())
  }
  return(fromJSON(file_path))
}

#' Resolve the full lineage for a given file
#'
#' @param filepath The path to the file.
#' @param project_root The path to the project root.
#' @return A list of trace nodes representing the lineage.
resolve_lineage_r <- function(filepath, project_root) {
  index <- load_trace_index(project_root)
  file_checksum <- digest::digest(filepath, algo="sha256", file=TRUE)
  
  if (!file_checksum %in% names(index)) {
    warning("No provenance found for file.")
    return(list())
  }
  
  current_trace_id <- index[[file_checksum]]
  lineage <- list()
  
  while (!is.null(current_trace_id) && current_trace_id != "null" && current_trace_id != "") {
    node <- load_trace_node(current_trace_id, project_root)
    lineage <- c(list(node), lineage) # Add to the front to maintain order from root to current
    current_trace_id <- node$parent
  }
  return(lineage)
}