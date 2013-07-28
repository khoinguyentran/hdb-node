#!/bin/sh
echo "Running post-update..."

# Resume supervising nodeapp
svc -u ~/services/nodeapp
