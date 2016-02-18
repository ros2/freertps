#!/usr/bin/env python

import os, sys, re
try:
  import rosidl_parser
except ImportError:
  print("\n\033[93mUnable to import the rosidl_parser module. Perhaps you haven't yet sourced the \ninstall/setup.bash file in your ros2 workspace?\033[0m\n\n")
  sys.exit(1)

def enforce_alignment(cur_alignment, required_alignment, f, indent=2):
  if required_alignment > cur_alignment:
    f.write("{0}if ((uintptr_t)_p & {1})\n".format(" "*indent, required_alignment-1))
    f.write("{0}  _p += {1} - ((uintptr_t)_p & {2});\n".format(" "*indent, required_alignment, required_alignment-1))

def enforce_read_alignment(cur_alignment, required_alignment, f, indent=2):
  if required_alignment > cur_alignment:
    spaces = " "*indent
    f.write("{0}if ((uintptr_t)*_p & {1})\n".format(spaces, required_alignment-1))
    f.write("{0}{{\n".format(spaces))
    f.write("{0}  _len -= {1} - ((uintptr_t)*_p & {2});\n".format(spaces, required_alignment, required_alignment-1))
    f.write("{0}  *_p += {1} - ((uintptr_t)*_p & {2});\n".format(spaces, required_alignment, required_alignment-1))
    f.write("{0}}}\n".format(spaces))

class PrimitiveType(object):
  def __init__(self, type_name, align):
    self.name = type_name
    self.align = align
  def serialize(self, name, cur_alignment, f):
    raise RuntimeError("serialization of {0} not implemented!".format(self.name))
  def deserialize(self, name, cur_alignment, f):
    #raise RuntimeError("deserialization of {0} not implemented!".format(self.name))
    f.write("  printf(\"deserialization of {0} not implemented!\");\n".format(self.name))
    f.write("  exit(1);\n")

class BooleanType(PrimitiveType):
  def __init__(self):
    super(BooleanType, self).__init__('bool', 1)
  def serialize(self, field_name, cur_alignment, f):
    f.write("  *_p = _s->{0} ? 1 : 0;\n".format(field_name))
    f.write("  _p++;\n")
  def serialize_fixed_array(self, field_name, cur_alignment, array_size, f):
    # this could be improved a lot
    f.write("  for (int _{0}_idx = 0; _{0}_idx < {1}; _{0}_idx++)\n".format(field_name, array_size))
    f.write("  {\n")
    f.write("    *_p = _s->{0}[_{0}_idx];\n".format(field_name))
    f.write("    _p++;\n")
    f.write("  }\n")
  def deserialize(self, field_name, cur_alignment, f):
    f.write("  _s->{0} = (**_p != 0);\n".format(field_name))
    f.write("  *_p += 1;\n") #(*_p)++;\n")
    f.write("  _len--;\n")

class NumericType(PrimitiveType):
  def __init__(self, c_name, align):
    super(NumericType, self).__init__(c_name, align)
  def serialize(self, field_name, cur_alignment, f):
    enforce_alignment(cur_alignment, self.align, f)
    f.write("  *(({0} *)_p) = _s->{1};\n".format(self.name, field_name))
    f.write("  _p += sizeof({0});\n".format(self.name))
  def serialize_fixed_array(self, field_name, cur_alignment, array_size, f):
    # this could be improved a lot
    enforce_alignment(cur_alignment, self.align, f)
    f.write("  for (uint32_t _{0}_idx = 0; _{0}_idx < {1}; _{0}_idx++)\n".format(field_name, array_size))
    f.write("  {\n")
    f.write("    *(({0} *)_p) = _s->{1}[_{1}_idx];\n".format(self.name, field_name))
    f.write("    _p += sizeof({0});\n".format(self.name))
    f.write("  }\n")
  def serialize_variable_array(self, field_name, cur_alignment, f):
    enforce_alignment(cur_alignment, 4, f) # align for the sequence count
    size_var = "_s->{0}.size".format(field_name)
    f.write("  *((uint32_t *)_p) = {0};\n".format(size_var))
    f.write("  _p += 4;\n")
    f.write("  memcpy(_p, _s->{0}.data, {1} * sizeof({1}));\n".format(field_name, size_var, self.name))
    f.write("  _p += {0} * sizeof({1});\n".format(size_var, self.name))
  def deserialize(self, field_name, cur_alignment, f):
    f.write("  if (*_len < sizeof({0}))\n    return false;\n".format(self.name))
    f.write("  _s->{0} = *(({1} *)*_p);\n".format(field_name, self.name))
    f.write("  *_p += sizeof({0});\n".format(self.name))
    f.write("  *_len -= sizeof({0});\n".format(self.name))
  def deserialize_fixed_array(self, field_name, cur_alignment, array_size, f):
    # this could be improved a lot
    enforce_read_alignment(cur_alignment, self.align, f)
    f.write("  for (uint32_t _{0}_idx = 0; _{0}_idx < {1}; _{0}_idx++)\n".format(field_name, array_size))
    f.write("  {\n")
    f.write("    _s->{1}[_{1}_idx] = *(({0} *)*_p);\n".format(self.name, field_name))
    f.write("    *_p += sizeof({0});\n".format(self.name))
    f.write("  }\n")
  def deserialize_variable_array(self, field_name, cur_alignment, f):
    f.write("  printf(\"this is not yet complete.\");\n")
    f.write("  return false;\n")
    #f.write("  if (*_len < sizeof({0}))\n    return false;\n".format(self.name))
    #f.write("  _s->{0} = *(({1} *)_p);\n".format(field_name, self.name))
    #f.write("  **_p += sizeof({0});\n".format(self.name))
    #f.write("  *_len -= sizeof({0});\n".format(self.name))

class StringType(PrimitiveType):
  def __init__(self):
    super(StringType, self).__init__('char *', 1)
  def serialize(self, field_name, cur_alignment, f):
    f.write("  uint32_t _{0}_len = (uint32_t)strlen(_s->{0}) + 1;\n".format(field_name))
    f.write("  *((uint32_t *)_p) = _{0}_len;\n".format(field_name))
    f.write("  _p += 4;\n")
    f.write("  memcpy(_p, _s->{0}, _{0}_len);\n".format(field_name))
    f.write("  _p += _{0}_len;\n".format(field_name))
  def serialize_variable_array(self, field_name, cur_alignment, f):
    enforce_alignment(cur_alignment, 4, f) # align for the sequence count
    size_var = "_s->{0}.size".format(field_name)
    f.write("  *((uint32_t *)_p) = {0};\n".format(size_var))
    f.write("  _p += 4;\n")
    f.write("  for (uint32_t _{0}_idx = 0; _{0}_idx < {1}; _{0}_idx++)\n".format(field_name, size_var))
    f.write("  {\n")
    enforce_alignment(cur_alignment, 4, f, indent=4)
    f.write("    size_t len = strlen(_s->{0}.data[_{0}_idx]) + 1;\n".format(field_name))
    f.write("    _p += 4;\n")
    f.write("    memcpy(_p, _s->{0}.data[_{0}_idx], len);\n".format(field_name))
    f.write("    _p += len;\n".format(field_name))
    f.write("  }\n")

  def deserialize_variable_array(self, field_name, cur_alignment, f):
    f.write("  printf(\"deserialization of variable-length string arrays not implemented!\\n\");")
    f.write("  exit(1);\n")


primitive_types = { }
primitive_types['bool']    = BooleanType()
primitive_types['byte']    = NumericType('uint8_t' , 1)
primitive_types['uint8']   = NumericType('uint8_t' , 1)
primitive_types['char']    = NumericType('int8_t'  , 1)
primitive_types['int8']    = NumericType('int8_t'  , 1)
primitive_types['uint16']  = NumericType('uint16_t', 2)
primitive_types['int16']   = NumericType('int16_t' , 2)
primitive_types['uint32']  = NumericType('uint32_t', 4)
primitive_types['int32']   = NumericType('int32_t' , 4)
primitive_types['uint64']  = NumericType('uint64_t', 8)
primitive_types['int64']   = NumericType('int64_t' , 8)
primitive_types['float32'] = NumericType('float'   , 4)
primitive_types['float64'] = NumericType('double'  , 8)
primitive_types['string']  = StringType()

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

  #if rosidl_type.pkg_name:
  #  print "   context for %s = %s" % (rosidl_type.type, rosidl_type.pkg_name)
  #return "ahhhh___%s" % rosidl_type.type

def c_includes(msg_spec):
  types = []
  for field in msg_spec.fields:
    if not field.type.type in primitive_types:
      lcase_type = uncamelcase(field.type.type)
      include = "%s/%s.h" % (field.type.pkg_name, lcase_type)
      if not include in types:
        types.append(include)
  return types

def serialize_field(field, align, f):
  typename = field.type.type
  if typename in primitive_types:
    if not field.type.is_array:
      primitive_types[typename].serialize(field.name, align, sf)
      align = primitive_types[typename].align
    elif field.type.array_size:
      primitive_types[typename].serialize_fixed_array(field.name, align, field.type.array_size, sf)
      # re-compute where we stand now in alignment, to save unnecessary if()
      align = primitive_types[typename].align * field.type.array_size
      if align % 8 == 0:
        align = 8
      elif align % 4 == 0:
        align = 4
      elif align % 2 == 0:
        align = 2
      else:
        align = 1
    else:
      primitive_types[typename].serialize_variable_array(field.name, align, sf)
      align = primitive_types[typename].align
  else:
    field_c_struct_name = "{0}__{1}".format(field.type.pkg_name, uncamelcase(field.type.type)).lower()
    if not field.type.is_array:
      sf.write("  _p += serialize_{0}(&_s->{1}, _p, _buf_size - (_p - _buf));\n".format(field_c_struct_name, field.name))
    elif field.type.array_size:
      sf.write("  for (uint32_t _{0}_idx = 0; _{0}_idx < {1}; _{0}_idx++)\n".format(field.name, field.type.array_size))
      sf.write("    _p += serialize_{0}(&_s->{1}[_{1}_idx], _p, _buf_size - (_p - _buf));\n".format(field_c_struct_name, field.name))
    else: # variable-length array of structs.
      # todo: i think the sequence length needs to be serialized first...
      sf.write("  for (uint32_t _{0}_idx = 0; _{0}_idx < _s->{0}.size; _{0}_idx++)\n".format(field.name))
      sf.write("    _p += serialize_{0}(&_s->{1}.data[_{1}_idx], _p, _buf_size - (_p - _buf));\n".format(field_c_struct_name, field.name))
    align = 1
  return align

def deserialize_field(field, align, f):
  typename = field.type.type
  if typename in primitive_types:
    if not field.type.is_array:
      primitive_types[typename].deserialize(field.name, align, sf)
      align = primitive_types[typename].align
    elif field.type.array_size:
      primitive_types[typename].deserialize_fixed_array(field.name, align, field.type.array_size, sf)
      # re-compute where we stand now in alignment, to save unnecessary if()
      align = primitive_types[typename].align * field.type.array_size
      if align % 8 == 0:
        align = 8
      elif align % 4 == 0:
        align = 4
      elif align % 2 == 0:
        align = 2
      else:
        align = 1
    else:
      primitive_types[typename].deserialize_variable_array(field.name, align, sf)
      align = primitive_types[typename].align
  else:
    field_c_struct_name = "{0}__{1}".format(field.type.pkg_name, uncamelcase(field.type.type)).lower()
    if not field.type.is_array:
      sf.write("  if (!deserialize_{0}(_p, _len, &_s->{1}))\n    return false;\n".format(field_c_struct_name, field.name))
    elif field.type.array_size:
      sf.write("  for (uint32_t _{0}_idx = 0; _{0}_idx < {1}; _{0}_idx++)\n".format(field.name, field.type.array_size))
      sf.write("    if (!deserialize_{0}(_p, _len, &_s->{1}[_{1}_idx]))\n    return false;\n".format(field_c_struct_name, field.name))
    else: # variable-length array of structs.
      sf.write("  printf(\"hmm... not sure how to deserialize dynamic arrays just yet.\");\n;  exit(1);\n")
      #sf.write("  for (uint32_t _{0}_idx = 0; _{0}_idx < _s->_{0}_size; _{0}_idx++)\n".format(field.name))
      #sf.write("    if (!deserialize_{0}(&_s->{1}[_{1}_idx], _p, _buf_size - (_p - _buf)))\n    return false;\n".format(field_c_struct_name, field.name))
    align = 1
  return align

################################################3
# begin global code awesomeness
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
      
      ############################################################
      struct_type = "%s__%s" % (pkg_name, uncamelcase(msg_name))
      hf.write("typedef struct %s\n{\n" % struct_type)
      for field in msg_spec.fields:
        print("    " + field.type.type + " " + field.name)
        c_typename = ""
        #if not field.type.is_array:

        if field.type.type in primitive_types:
          c_typename = primitive_types[field.type.type].name
        else:
          c_typename = "struct {0}__{1}".format(field.type.pkg_name, uncamelcase(field.type.type)).lower()

        if field.type.is_array and not field.type.array_size:
          if field.type.type in primitive_types:
            hf.write("  struct freertps__{0}__array {1};\n".format(field.type.type, field.name))
          else:
            hf.write("  struct freertps__{0}__{1}__array {2};\n".format(field.type.pkg_name, uncamelcase(field.type.type), field.name).lower())
        elif field.type.is_array:
          hf.write("  {0} {1}[{2}];\n".format(c_typename, field.name, field.type.array_size))
        else:
          hf.write("  {0} {1};\n".format(c_typename, field.name))

      hf.write("} %s_t;\n\n" % struct_type)
      type_obj_name = "%s__%s__type" % (pkg_name, uncamelcase(msg_name))
      hf.write("extern const struct freertps_type %s;\n\n" % type_obj_name)
      hf.write("uint32_t serialize_%s(void *_msg, uint8_t *_buf, uint32_t _buf_size);\n" % struct_type)
      for field in msg_spec.fields[1:]:
        hf.write("uint32_t serialize_{0}__until_{1}(void *_msg, uint8_t *_buf, uint32_t _buf_size);\n".format(struct_type, field.name))
      hf.write("bool deserialize_%s(uint8_t **_p, uint32_t *_len, void *_msg);\n" % struct_type)
      #for field in msg_spec.fields[1:]:
      #  hf.write("bool deserialize_{0}__until_{1}(void *_msg, uint8_t *_buf, uint32_t _buf_size);\n".format(struct_type, field.name))
      hf.write("\nFREERTPS_ARRAY({0}, {0}_t);\n".format(struct_type))
      hf.write("\n#endif\n")
      ####################
      source_fn = os.path.join(msg_tree_root, 'src', struct_type) + '.c'
      sf = open(source_fn, 'w')
      sf.write("#include \"freertps/type.h\"\n")
      sf.write("#include <string.h>\n")
      sf.write("#include \"%s\"\n\n" % os.path.join(pkg_name, header_fn))
      ### first, emit the whole-enchilada serialization function
      sf.write("uint32_t serialize_%s(void *_msg, uint8_t *_buf, uint32_t _buf_size)\n{\n" % struct_type)
      sf.write("  struct %s *_s = (struct %s *)_msg;\n" % (struct_type, struct_type))
      sf.write("  uint8_t *_p = _buf;\n")
      enforce_alignment(1, 4, sf)
      align = 4
      for field in msg_spec.fields:
        align = serialize_field(field, align, sf)
      sf.write("  return _p - _buf;\n")
      sf.write("}\n\n")
      ### now, emit the partial serialization functions
      for until_field in msg_spec.fields[1:]:
        sf.write("uint32_t serialize_{0}__until_{1}(void *_msg, uint8_t *_buf, uint32_t _buf_size)\n{{\n".format(struct_type, until_field.name))
        sf.write("  struct {0} *_s = (struct {0} *)_msg;\n".format(struct_type))
        sf.write("  uint8_t *_p = _buf;\n")
        enforce_alignment(1, 4, sf)
        align = 4
        for field in msg_spec.fields:
          if field == until_field:
            break
          align = serialize_field(field, align, sf)
        sf.write("  return _p - _buf;\n")
        sf.write("}\n\n")

      ### emit the whole-enchilada deserialization function
      sf.write("bool deserialize_%s(uint8_t **_p, uint32_t *_len, void *_msg)\n{\n" % struct_type)
      sf.write("  struct {0} *_s = (struct {0} *)_msg;\n".format(struct_type))
      #sf.write("  uint8_t *_p = _buf;\n")
      enforce_read_alignment(1, 4, sf)
      align = 4
      for field in msg_spec.fields:
        align = deserialize_field(field, align, sf)
      sf.write("  return true;\n") #_rpos - _buf;\n")
      sf.write("}\n\n")
      '''
      ### now, emit the partial serialization functions
      for until_field in msg_spec.fields[1:]:
        sf.write("bool deserialize_{0}__until_{1}(void *_msg, uint8_t *_buf, uint32_t _buf_size)\n{{\n".format(struct_type, until_field.name))
        sf.write("  struct {0} *_s = (struct {0} *)_msg;\n".format(struct_type))
        sf.write("  uint8_t *_p = _buf;\n")
        enforce_alignment(1, 4, sf)
        align = 4
        for field in msg_spec.fields:
          if field == until_field:
            break
          align = deserialize_field(field, align, sf) sf.write("  return true;\n") #return _wpos - _buf;\n")
        sf.write("}\n\n")
      '''
 
      sf.write("const struct freertps_type %s =\n" % type_obj_name)
      sf.write("{\n");
      sf.write("  .rtps_typename = \"{0}::msg::dds_::{1}_\",\n".format(pkg_name, msg_name))
      sf.write("  .serialize = serialize_{0}\n".format(struct_type))
      sf.write("};\n")
