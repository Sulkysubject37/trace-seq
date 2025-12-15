#!/bin/bash

# This script demonstrates a mock RNA-seq pipeline with TRACE-SEQ.
# Note: This script is for demonstration purposes only and does not
# perform any real data processing. The C++ core and Python CLI
# must be compiled and installed for this script to run.

# Step 1: Initial annotation of raw counts
echo "Step 1: Annotating raw counts"
traceseq annotate examples/rnaseq/raw_counts.tsv \
    --operation aggregation \
    --method "featureCounts" \
    --assumption "reference_version:hg38"

# Step 2: TPM normalization
echo "Step 2: Normalizing to TPM"
# Create a dummy normalized file
cp examples/rnaseq/raw_counts.tsv examples/rnaseq/tpm.tsv
traceseq annotate examples/rnaseq/tpm.tsv \
    --operation normalization \
    --method "TPM" \
    --assumption "library_preparation:polyA_selected"

# Step 3: Filtering
echo "Step 3: Filtering low-expressed genes"
# Create a dummy filtered file
cp examples/rnaseq/tpm.tsv examples/rnaseq/filtered.tsv
traceseq annotate examples/rnaseq/filtered.tsv \
    --operation filtering \
    --method "threshold" \
    --assumption "sequencing_depth:deep"

# Step 4: Log transform
echo "Step 4: Log transforming data"
# Create a dummy transformed file
cp examples/rnaseq/filtered.tsv examples/rnaseq/log_transformed.tsv
traceseq annotate examples/rnaseq/log_transformed.tsv \
    --operation transformation \
    --method "log2" \
    --assumption "model_assumption:normality"

echo "Pipeline complete. Use 'traceseq explain' to view the provenance of each file."
