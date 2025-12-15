// This file will contain the Python bindings for the traceseq library.
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include "tracer.hpp"
#include "lineage.hpp"
#include "hashing.hpp"
#include "pybind11_json.hpp"

namespace py = pybind11;

PYBIND11_MODULE(traceseq_py, m) {
    m.doc() = "Python bindings for the traceseq library";

    py::class_<TraceNode::Operation>(m, "Operation")
        .def(py::init<>()) 
        .def_readwrite("op_class", &TraceNode::Operation::op_class)
        .def_readwrite("method", &TraceNode::Operation::method)
        .def_readwrite("parameters", &TraceNode::Operation::parameters);

    py::class_<TraceNode::Input>(m, "Input")
        .def(py::init<>()) 
        .def_readwrite("shape", &TraceNode::Input::shape)
        .def_readwrite("checksum", &TraceNode::Input::checksum);

    py::class_<TraceNode::Output>(m, "Output")
        .def(py::init<>()) 
        .def_readwrite("data_class", &TraceNode::Output::data_class)
        .def_readwrite("unit", &TraceNode::Output::unit)
        .def_readwrite("checksum", &TraceNode::Output::checksum);

    py::class_<TraceNode::Environment>(m, "Environment")
        .def(py::init<>()) 
        .def_readwrite("language", &TraceNode::Environment::language)
        .def_readwrite("tool", &TraceNode::Environment::tool)
        .def_readwrite("version", &TraceNode::Environment::version);

    py::class_<TraceNode>(m, "TraceNode")
        .def(py::init<>()) 
        .def_readwrite("trace_id", &TraceNode::trace_id)
        .def_readwrite("parent", &TraceNode::parent)
        .def_readwrite("timestamp", &TraceNode::timestamp)
        .def_readwrite("data_class", &TraceNode::data_class)
        .def_readwrite("operation", &TraceNode::operation)
        .def_readwrite("assumptions", &TraceNode::assumptions)
        .def_readwrite("input", &TraceNode::input)
        .def_readwrite("output", &TraceNode::output)
        .def_readwrite("environment", &TraceNode::environment)
        .def_readwrite("ontology_version", &TraceNode::ontology_version)
        .def("save", &TraceNode::save);
    
    py::class_<Ontology>(m, "Ontology")
        .def(py::init<>()) 
        .def("load", &Ontology::load)
        .def("validate_operation", &Ontology::validate_operation)
        .def("validate_assumption", &Ontology::validate_assumption);

    m.def("create_trace_node", &create_trace_node, "Create a new trace node");
    m.def("validate_node", &validate_node, "Validate a trace node");
    m.def("load_index", &load_index, "Load the trace index");
    m.def("save_index", &save_index, "Save the trace index");
    m.def("resolve_lineage", &resolve_lineage, "Resolve the full lineage for a given file");
    m.def("sha256_file", &sha256_file, "Calculate the SHA256 checksum of a file");
}
