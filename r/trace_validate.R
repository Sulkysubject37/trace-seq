library(testthat)

#' Validate a single trace node
#'
#' @param node The trace node to validate.
#' @param ontologies A list containing the operation and assumption ontologies.
#' @return TRUE if the node is valid, otherwise throws an error.
validate_trace_node <- function(node, ontologies) {
  # Check for required fields
  required_fields <- c("trace_id", "parent", "timestamp", "data_class", "operation", "assumptions", "input", "output", "environment", "ontology_version")
  for (field in required_fields) {
    expect_true(field %in% names(node), paste("Missing required field:", field))
  }
  
  # Validate operation
  op_class <- node$operation$class
  expect_true(op_class %in% names(ontologies$operations$operation_classes), "Invalid operation class")
  
  # Validate assumptions
  for (assumption in node$assumptions) {
    # This is a simplification. A real implementation would check the assumption ID against the ontology.
    expect_true(is.character(assumption), "Invalid assumption format")
  }
  
  return(TRUE)
}

#' Validate a full lineage
#'
#' @param lineage A list of trace nodes.
#' @param ontologies A list containing the operation and assumption ontologies.
#' @return TRUE if the lineage is valid, otherwise throws an error.
validate_lineage <- function(lineage, ontologies) {
  for (i in 1:length(lineage)) {
    node <- lineage[[i]]
    validate_trace_node(node, ontologies)
    if (i > 1) {
      # Check that the parent ID matches the previous node's ID
      expect_equal(node$parent, lineage[[i-1]]$trace_id, "Broken lineage: parent ID mismatch")
    }
  }
  return(TRUE)
}
