import datetime
import configparser
import os
import subprocess

Import("env")

revision = (
    subprocess.check_output(["git", "rev-parse", "--short", "HEAD"])
    .strip()
    .decode("utf-8")
)
curr_date = datetime.datetime.now()
date_str = f"{curr_date.year}-{curr_date.month:02}-{curr_date.day:02}"
time_str = f"{curr_date.hour:02}:{curr_date.minute:02}:{curr_date.second:02}"

env.Append(
    CPPDEFINES=[
        ("PIO_SRC_REV", '\\"' + revision + '\\"'),
        ("PIO_BUILD_DATE", '\\"' + date_str + '\\"'),
        ("PIO_BUILD_TIME", '\\"' + time_str + '\\"'),
    ]
)

config = configparser.ConfigParser(interpolation=None)
config.read(os.path.join(env["PROJECT_DIR"], "platformio.ini"))
major = config.get("version", "major", fallback="0").strip()
minor = config.get("version", "minor", fallback="0").strip()
build = config.get("version", "build", fallback="0").strip()
version_string = f"{major}.{minor}.{build}"

board_name = env["BOARD"]
env.Replace(PROGNAME="mavesp-{}-{}".format(board_name, version_string))
