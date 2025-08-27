# MACSM Manager

The MACSM Manager app provides runtime load/unload/reload control of cFS applications using core Flight Executive (cFE) Executive Services APIs. The app listens for command messages on the software bus and invokes ES calls to manage other applications.

## Commands

| Command | Function Code | Payload |
|---------|---------------|---------|
| Unload  | 1 | `char Name[OS_MAX_API_NAME]` |
| Load    | 2 | `char Name[OS_MAX_API_NAME]; char Entry[OS_MAX_API_NAME]; char Path[OS_MAX_PATH_LEN]; uint32 Stack; uint32 Priority;` |
| Reload  | 3 | `char Name[OS_MAX_API_NAME]; char Path[OS_MAX_PATH_LEN];` |

## Example Usage

From the repo root, run the helper scripts which forward commands via UDP to the flight software:

```bash
./macsm_unload.sh sample_app
./macsm_load.sh sample_app sample_app_Main /path/to/sample_app.so 16384 50
./macsm_reload.sh sample_app /path/to/sample_app.so
```

Each script emits an event through cFE Event Services indicating success or failure.

## Limitations

* A small denylist prevents unloading core services such as ES, SB, TBL, TIME, and the manager itself.
* String fields are capped at `OS_MAX_API_NAME` for names and entry points and `OS_MAX_PATH_LEN` for paths.
* The ground script assumes commands are routed through `ci_lab` on localhost UDP port 1234.
