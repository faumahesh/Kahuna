import datetime
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

my_flags = env.ParseFlags(env["BUILD_FLAGS"])
defines = dict(my_flags.get("CPPDEFINES") or [])

version_string = defines.get("VERSION_STRING")
if version_string is None:
    version_string = "0.0.0"

board_name = env["BOARD"]
env.Replace(PROGNAME="mavesp-{}-{}".format(board_name, version_string))
