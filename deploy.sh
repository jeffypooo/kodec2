#!/usr/bin/env bash
echo "Building..."
echo
./gradlew lib:install
echo
echo "Deploying..."
echo
./gradlew lib:bintrayUpload -DBINTRAY_USER=${BINTRAY_USER} -DBINTRAY_KEY=${BINTRAY_KEY}
echo
echo "Done!"