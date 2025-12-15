library(yaml)

#' Load the operation ontology
#'
#' @param project_root The path to the project root.
#' @return A list representing the operation ontology.
load_operation_ontology <- function(project_root) {
  CORE_PATH <- file.path(project_root, "core")
  file_path <- file.path(CORE_PATH, "operation_ontology.yaml")
  if (!file.exists(file_path)) {
    stop("Operation ontology not found.")
  }
  return(read_yaml(file_path))
}

#' Load the assumption ontology
#'
#' @param project_root The path to the project root.
#' @return A list representing the assumption ontology.
load_assumption_ontology <- function(project_root) {
  CORE_PATH <- file.path(project_root, "core")
  file_path <- file.path(CORE_PATH, "assumption_ontology.yaml")
  if (!file.exists(file_path)) {
    stop("Assumption ontology not found.")
  }
  return(read_yaml(file_path))
}

#' Validate an operation class
#'
#' @param op_class The operation class to validate.
#' @param ontologies A list containing the operation and assumption ontologies.
#' @return TRUE if the operation is valid, otherwise throws an error.
validate_operation <- function(op_class, ontologies) {
  if (!op_class %in% names(ontologies$operations$operation_classes)) {
    stop("Invalid operation class")
  }
  return(TRUE)
}

#' Validate an assumption
#'
#' @param assumption The assumption to validate.
#' @param ontologies A list containing the operation and assumption ontologies.
#' @return TRUE if the assumption is valid, otherwise throws an error.
validate_assumption <- function(assumption, ontologies) {
    # This is a simplification. A real implementation would check the assumption ID against the ontology.
    if (!is.character(assumption)) {
        stop("Invalid assumption format")
    }
  return(TRUE)
}
