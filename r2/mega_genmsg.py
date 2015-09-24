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

def uncamelcase(camelcase):
  lower = ""
  upper_run_len = 0
  for idx in xrange(0, len(camelcase)):
    #print(camelcase[idx])
    if (camelcase[idx].isupper()):
      next_lower = idx < len(camelcase)-1 and camelcase[idx+1].islower()
      if (idx > 0 and upper_run_len == 0) or (idx > 1 and next_lower):
        lower += '_'
      lower += camelcase[idx].lower()
      upper_run_len += 1
    else:
      lower += camelcase[idx]
      upper_run_len = 0
  return lower

def print_uncamelcase(c):
  print("%s -> %s" % (c, uncamelcase(c)))
  
def camelcase_to_lower_samples():
  print_uncamelcase("String")
  print_uncamelcase("UInt32")
  print_uncamelcase("UInt32MultiArray")
  print_uncamelcase("MultiArrayLayout")
  print_uncamelcase("NavSatStatus")
  print_uncamelcase("MultiDOFJointState")
  print_uncamelcase("RegionOfInterest")
  print_uncamelcase("PointCloud2")
  print_uncamelcase("PointField")
  print_uncamelcase("MultiEchoLaserScan")

def c_type(rosidl_type):
  if rosidl_type.type in primitive_type_map:
    c_typename = primitive_type_map[rosidl_type.type]
  else:
    lcase_type = uncamelcase(rosidl_type.type)
    c_typename = ("struct %s__%s" % (rosidl_type.pkg_name, lcase_type)).lower()
  return c_typename

def c_decl_suffix(rosidl_type):
  if rosidl_type.is_array and rosidl_type.array_size:
    return "[%d]" % rosidl_type.array_size
  return ""

  #if rosidl_type.pkg_name:
  #  print "   context for %s = %s" % (rosidl_type.type, rosidl_type.pkg_name)
  #return "ahhhh___%s" % rosidl_type.type

def c_includes(msg_spec):
  types = []
  for field in msg_spec.fields:
    if not field.type.type in primitive_type_map:
      lcase_type = uncamelcase(field.type.type)
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
  os.makedirs(os.path.join(msg_tree_root,'src'))
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
      header_fn = uncamelcase(msg_name) + '.h'
      header_path = os.path.join(pkg_output_path, header_fn)
      hf = open(header_path, 'w')
      include_guard = ("R2_%s__%s" % (pkg_name, uncamelcase(msg_name))).upper()
      hf.write("#ifndef %s\n" % include_guard)
      hf.write("#define %s\n\n" % include_guard)
      hf.write("#include <stdint.h>\n")
      hf.write("#include <stdbool.h>\n")
      hf.write("#include \"freertps/type.h\"\n")
      includes = c_includes(msg_spec)
      if includes:
        for include in includes:
          hf.write("#include \"%s\"\n" % include)
      hf.write("\n")
      
      struct_type = "%s__%s" % (pkg_name, uncamelcase(msg_name))
      hf.write("#define _%s_RTPS_TYPENAME \"%s::msg::dds_::%s_\"\n\n" % (struct_type, pkg_name, msg_name))

      hf.write("typedef struct %s\n{\n" % struct_type)
      for field in msg_spec.fields:
        print("    " + field.type.type + " " + field.name)
        hf.write("  %s %s%s;\n" % (c_type(field.type), field.name, c_decl_suffix(field.type)))
      hf.write("} %s_t;\n\n" % struct_type)
      type_obj_name = "%s__%s_" % (pkg_name, uncamelcase(msg_name))
      hf.write("extern const struct freertps_type %s;\n\n" % type_obj_name)
      hf.write("#endif\n")
      ####################
      source_fn = os.path.join(msg_tree_root, 'src', struct_type) + '.c'
      sf = open(source_fn, 'w')
      sf.write("#include \"freertps/type.h\"\n")
      sf.write("#include <string.h>\n")
      sf.write("#include \"%s\"\n\n" % os.path.join(pkg_name, header_fn))
      sf.write("uint32_t serialize_%s(void *msg, uint8_t *buf, uint32_t buf_size)\n{\n" % struct_type)
      sf.write("  struct %s *p = (struct %s *)msg;\n" % (struct_type, struct_type))
      sf.write("  uint8_t *wpos = buf;\n")
      for field in msg_spec.fields:
        sf.write("  memcpy(wpos, &p->%s, sizeof(p->%s));\n" % (field.name, field.name));
        sf.write("  wpos += sizeof(p->%s);\n" % field.name)
      sf.write("  return 0;\n")
      sf.write("}\n\n")
      sf.write("const struct freertps_type %s =\n" % type_obj_name)
      sf.write("{\n");
      sf.write("  .rtps_typename = \"%s::msg::dds_::%s_\"\n" % (pkg_name, msg_name))
      sf.write("};\n")
