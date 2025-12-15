# TRACE-SEQ Ontology

The TRACE-SEQ ontology provides a fixed vocabulary for describing operations and assumptions. This ensures that all provenance information is consistent and comparable.

## Operation Classes

| Class           | Description                               |
|-----------------|-------------------------------------------|
| `normalization`   | Rescaling data to a common scale.         |
| `filtering`       | Removing data points based on criteria.   |
| `transformation`  | Applying a mathematical function.         |
| `aggregation`     | Summarizing data.                         |
| `inference`       | Inferring biological meaning.             |
| `annotation`      | Adding metadata to the data.              |
| `selection`       | Selecting a subset of data.               |

## Assumption Classes

| Class                 | Description                                       |
|-----------------------|---------------------------------------------------|
| `library_preparation` | How the sequencing library was generated.         |
| `batch_correction`    | How batch effects were handled.                   |
| `reference_version`   | The reference genome or transcriptome version.    |
| `sequencing_depth`    | The depth of sequencing.                          |
| `model_assumption`    | Assumptions made by a statistical model.          |
