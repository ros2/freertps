#!/usr/bin/env python

import os, sys, re
try:
  import rosidl_parser
except ImportError:
  print("\n\033[93mUnable to import the rosidl_parser module. Perhaps you haven't yet sourced the \ninstall/setup.bash file in your ros2 workspace?\033[0m\n\n")
  sys.exit(1)

primitive_type_map = {
  'bool'    : 'bool',
  'byte'    : 'uint8_t',
  'char'    : 'char',
  'float32' : 'float',
  'float64' : 'double',
  'int8'    : 'int8_t',
  'uint8'   : 'uint8_t',
  'int16'   : 'int16_t',
  'uint16'  : 'uint16_t',
  'int32'   : 'int32_t',
  'uint32'  : 'uint32_t',
  'int64'   : 'int64_t',
  'uint64'  : 'uint64_t',
  'string'  : 'char *'
}

def camelcase_to_underscores(camelcase):
  # from http://stackoverflow.com/questions/1175208/elegant-python-function-to-convert-camelcase-to-camel-case
  s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', camelcase)
  return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def c_type(rosidl_type):
  if rosidl_type.type in primitive_type_map:
    c_typename = primitive_type_map[rosidl_type.type]
  else:
    lcase_type = camelcase_to_underscores(rosidl_type.type)
    c_typename = ("%s_%s_t" % (rosidl_type.pkg_name, lcase_type)).lower()
  if rosidl_type.is_array and rosidl_type.array_size:
    c_typename += "[%d]" % rosidl_type.array_size
  return c_typename
  #if rosidl_type.pkg_name:
  #  print "   context for %s = %s" % (rosidl_type.type, rosidl_type.pkg_name)
  #return "ahhhh___%s" % rosidl_type.type

def c_includes(msg_spec):
  types = []
  for field in msg_spec.fields:
    if not field.type.type in primitive_type_map:
      lcase_type = camelcase_to_underscores(field.type.type)
      include = "%s/%s.h" % (field.type.pkg_name, lcase_type)
      if not include in types:
        types.append(include)
  return types

#        print("    " + field.type.type + " " + field.name)
#        of.write("  %s %s;\n" % (c_type(field.type), field.name))

ament_prefix = os.environ['AMENT_PREFIX_PATH']
ifaces_path = os.path.join(ament_prefix, 'share', 'ament_index', 'resource_index', 'rosidl_interfaces')
share_path = os.path.join(ament_prefix, 'share')
if not os.path.isdir(ifaces_path) or not os.path.isdir(share_path):
  print("ament_index for rosidl_interfaces seems to be empty. Perhaps this workspace hasn't been built yet?")
  sys.exit(1)

msg_tree_root = os.path.join('build','msgs')
if not os.path.exists(msg_tree_root):
  os.makedirs(msg_tree_root)
for pkg_name in os.listdir(ifaces_path):
  full_path = os.path.join(ifaces_path, pkg_name)
  pkg_output_path = os.path.join(msg_tree_root, pkg_name)
  pkg_share_path = os.path.join(share_path, pkg_name)
  if not os.path.exists(pkg_output_path):
    os.makedirs(pkg_output_path)
  with open(full_path) as f:
    for line in f:
      extension = line.rstrip().split('.')[-1]
      if extension == 'srv':
        continue
      msg_filename = os.path.join(pkg_share_path, 'msg', line.rstrip())
      msg_spec = rosidl_parser.parse_message_file(pkg_name, msg_filename)
      msg_name = '.'.join(line.rstrip().split('.')[0:-1])
      print("  %s/%s" % (pkg_name, msg_name))
      of = open(os.path.join(pkg_output_path, msg_name) + '.h', 'w')
      include_guard = ("R2_%s_%s" % (pkg_name, msg_name)).upper()
      of.write("#ifndef %s\n" % include_guard)
      of.write("#define %s\n\n" % include_guard)
      includes = c_includes(msg_spec)
      if includes:
        for include in includes:
          of.write("#include \"%s\"\n" % include)
      of.write("\n")
      struct_type = ("%s_%s_t" % (pkg_name, msg_name)).lower()
      of.write("typedef struct %s\n{\n" % struct_type)
      for field in msg_spec.fields:
        print("    " + field.type.type + " " + field.name)
        of.write("  %s %s;\n" % (c_type(field.type), field.name))
      of.write("}\n\n")
      of.write("#endif\n")
