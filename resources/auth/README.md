Use `example.json` for a credential-free build. To sign in locally, create an ignored
`secrets.json` with the same fields and configure CMake with
`-DYH_AUTH_CONFIG_FILE=/absolute/path/to/secrets.json`. The selected file is embedded
in the executable; do not publish a build containing real credentials.
