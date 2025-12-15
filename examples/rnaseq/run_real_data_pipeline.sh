#!/bin/bash

# This script demonstrates a mock pipeline with TRACE-SEQ on a real dataset.

# Get the absolute path to the traceseq executable
TRACESEQ_EXEC="/Users/sulky/Documents/trace-seq/cpp/build/traceseq"

# Step 1: Initial annotation of the raw data
echo "Step 1: Annotating raw data"
STEP1_OUTPUT=$("$TRACESEQ_EXEC" --annotate=GSE43335_smrna_locus.txt \
    --operation=annotation \
    --method=\"GEO\" \
    --assumption=data_source:GEO)
echo "$STEP1_OUTPUT"
PARENT_ID_1=$(echo "$STEP1_OUTPUT" | grep "trace ID" | cut -d ' ' -f 7)

# Step 2: Filtering the data
echo "Step 2: Filtering the data"
# Create a dummy filtered file
grep -v "^#" GSE43335_smrna_locus.txt | awk '$5 > 0' > filtered_loci.txt
STEP2_OUTPUT=$("$TRACESEQ_EXEC" --annotate=filtered_loci.txt \
    --operation=filtering \
    --method=\"awk\" \
    --parent="$PARENT_ID_1")
echo "$STEP2_OUTPUT"

echo "Pipeline complete. Use 'traceseq explain <file>' to view the provenance of each file."
