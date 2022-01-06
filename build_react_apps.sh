#! /bin/sh

cd ./asset/create-react-app

cd react-emoji-search
npm install
npm run build

cd ../react-ios-calculator
npm install
npm run build
