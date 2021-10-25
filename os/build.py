from os import listdir
from os.path import isfile, join

# Default
APP_PATH = '../user/src/bin/'
TARGET_PATH = '../user/build/'

# Tutorial
APP_PATH = '../../rCore-Tutorial-v3/user/src/bin/'
TARGET_PATH = '../../rCore-Tutorial-v3/user/target/riscv64gc-unknown-none-elf/release/'

# Test
# APP_PATH = '../../rCore_tutorial_tests/user/build/elf/'
# TARGET_PATH = '../../rCore_tutorial_tests/user/build/elf/'

ELF_SUFFIX = ""

if __name__ == '__main__':
  f = open('src/link_app.S', 'w')

  apps = [x.split('.')[0] for x in listdir(APP_PATH) if isfile(join(APP_PATH, x))]
  apps.sort()

  f.write(
"""
  .align 3
  .section .data
  .global _num_app
_num_app:
  .quad {}
""".format(len(apps))
  )

  for i in range(len(apps)):
    f.write("  .quad app_{}_start\n".format(i))
  f.write("  .quad app_{}_end\n".format(len(apps) - 1))

  f.write(
"""
  .global _app_names
_app_names:
"""
  )

  for i in range(len(apps)):
    f.write("  .string \"{}\"\n".format(apps[i]))

  for i in range(len(apps)):
    f.write(
"""
  .section .data
  .global app_{0}_start
  .global app_{0}_end
app_{0}_start:
  .incbin "{2}{1}{3}"
app_{0}_end:
""".format(i, apps[i], TARGET_PATH, ELF_SUFFIX)
    )

  f.close()
