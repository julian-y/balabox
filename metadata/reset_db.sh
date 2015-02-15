#!/bin/bash
# Drops metaserver mysql tables and readds them along with default test data
SCRIPT_DIR=./scripts
# drop tables
mysql test -u cs188 < $SCRIPT_DIR/drop.sql;

# create
mysql test -u cs188 < $SCRIPT_DIR/create.sql;

# add test data
mysql test -u cs188 < $SCRIPT_DIR/insert_test_rows.sql;
