#!/bin/sh
echo "Running pre-update..."

# Stop respawning nodeapp
svc -d ~/services/nodeapp