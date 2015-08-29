#!/usr/bin/env python

import os, sys
import rosidl_parser

ament_prefix = os.environ['AMENT_PREFIX_PATH']
ifaces_path = os.path.join(ament_prefix, 'share', 'ament_index', 'resource_index', 'rosidl_interfaces')
if not os.path.isdir(ifaces_path):
  print("ament_index for rosidl_interfaces seems to be empty. Perhaps this workspace hasn't been built yet?")
  sys.exit(1)
for pkg_name in os.listdir(ifaces_path):
  full_path = os.path.join(ifaces_path, pkg_name)
  print(full_path)
  with open(full_path) as f:
    for line in f:
      msg_name = line.rstrip()
      print("  %s/%s" % (pkg_name, msg_name))
 
#print(ifaces_path)

#ament_index_root = "~/ros2_ws/install/share/ament_index"
