from . import traceseq_py
import os

def get_project_root():
    current_dir = os.path.abspath(os.path.dirname(__file__))
    while not os.path.exists(os.path.join(current_dir, "README.md")):
        current_dir = os.path.dirname(current_dir)
        if current_dir == os.path.dirname(current_dir):
            return os.getcwd()
    return current_dir

def annotate(filepath, operation, method, assumptions=[], parent_id="null"):
    project_root = get_project_root()
    
    input_checksum = traceseq_py.sha256_file(filepath)

    ontology = traceseq_py.Ontology()
    op_ontology_path = os.path.join(project_root, "core", "operation_ontology.yaml")
    assump_ontology_path = os.path.join(project_root, "core", "assumption_ontology.yaml")
    ontology.load(op_ontology_path, assump_ontology_path)

    if not ontology.validate_operation(operation):
        raise ValueError(f"Invalid operation: {operation}")

    for assumption in assumptions:
        if not ontology.validate_assumption(assumption):
            raise ValueError(f"Invalid assumption: {assumption}")

    node = traceseq_py.create_trace_node(
        parent_id,
        "unknown", # data_class
        operation,
        method,
        assumptions
    )
    node.input.checksum = input_checksum
    node.output.data_class = "unknown"
    
    output_checksum = input_checksum # Placeholder
    node.save(input_checksum, output_checksum, "unknown", project_root)

    print(f"Successfully annotated {filepath} with trace ID: {node.trace_id}")
    return node.trace_id

def explain(filepath):
    project_root = get_project_root()
    input_checksum = traceseq_py.sha256_file(filepath)
    
    index = traceseq_py.load_index(project_root)
    if input_checksum not in index:
        return []
        
    trace_id = index[input_checksum]
    return traceseq_py.resolve_lineage(trace_id, project_root)
