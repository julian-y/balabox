#!/bin/bash
# Drops metaserver mysql tables and readds them along with default test data

# drop tables
mysql test < drop.sql;

# create
mysql test < create.sql;

# add test data
mysql test < insert_test_rows.sql;