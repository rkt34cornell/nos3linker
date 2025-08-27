#!/bin/bash
python3 tools/macsm/send_macsm_cmd.py --host "${MACSM_HOST:-127.0.0.1}" --port "${MACSM_PORT:-1234}" unload "$@"
