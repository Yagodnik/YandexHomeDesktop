python3 scripts/build-release.py
rm -rf deploy
mkdir deploy
cp -r build-release/YandexHomeDesktop.app deploy/
cd deploy
../scripts/macos-installer.sh