from os import listdir
from os.path import isfile, join

TARGET_PATH = '../user/build/'

if __name__ == '__main__':
  f = open('src/link_app.S', 'w')

  app_path = '../user/src/bin'
  apps = [x.split('.')[0] for x in listdir(app_path) if isfile(join(app_path, x))]
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

  for i in range(len(apps)):
    f.write(
"""
  .section .data
  .global app_{0}_start
  .global app_{0}_end
app_{0}_start:
  .incbin "{2}{1}.bin"
app_{0}_end:
""".format(i, apps[i], TARGET_PATH)
    )

  f.close()
