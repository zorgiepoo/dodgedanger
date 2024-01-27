#!/usr/bin/env bash
codesign -s "Developer ID" --force --timestamp -o runtime --entitlements DodgeDanger/DodgeDanger.entitlements "$1"
/usr/bin/ditto -c -k --keepParent "$1" "DodgeDanger.zip"
xcrun notarytool submit "DodgeDanger.zip" --keychain-profile "notarytool-password" --wait
