#!/usr/bin/env bash
codesign --force --timestamp -o runtime --entitlements DodgeDanger/DodgeDanger.entitlements "$1"
