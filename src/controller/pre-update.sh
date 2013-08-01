#!/bin/sh
echo "Running pre-update..."

# Stop respawning app
svc -d service/app