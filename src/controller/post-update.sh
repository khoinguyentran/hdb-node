#!/bin/sh
echo "Running post-update..."

# Resume supervising eapp
svc -u service/app
