#!/usr/bin/env python

import os, sys
import rosidl_parser

ament_prefix = os.environ['AMENT_PREFIX_PATH']
ifaces_path = os.path.join(ament_prefix, 'share', 'ament_index', 'resource_index', 'rosidl_interfaces')
if not os.path.isdir(ifaces_path):
  print("ament_index for rosidl_interfaces seems to be empty. Perhaps this workspace hasn't been built yet?")
  sys.exit(1)

msg_tree_root = os.path.join('build','msgs')
if not os.path.exists(msg_tree_root):
  os.makedirs(msg_tree_root)
for pkg_name in os.listdir(ifaces_path):
  full_path = os.path.join(ifaces_path, pkg_name)
  #print(full_path)
  pkg_output_path = os.path.join(msg_tree_root, pkg_name)
  if not os.path.exists(pkg_output_path):
    os.makedirs(pkg_output_path)
  with open(full_path) as f:
    for line in f:
      msg_filename = line.rstrip()
      msg_spec = rosidl_parser.parse_message_file(pkg_name, msg_filename)
      msg_name = '.'.join(line.rstrip().split('.')[0:-1])
      print("  %s/%s" % (pkg_name, msg_name))
      of = open(os.path.join(pkg_output_path, msg_name) + '.h', 'w')
      include_guard = ("R2_%s_%s" % (pkg_name, msg_name)).upper()
      of.write("#ifndef %s\n" % include_guard)
      of.write("#define %s\n\n" % include_guard)
      struct_type = ("%s_%s_t" % (pkg_name, msg_name)).lower()
      of.write("typedef struct %s\n{\n" % struct_type)
      of.write("}\n\n")
      of.write("#endif\n")
