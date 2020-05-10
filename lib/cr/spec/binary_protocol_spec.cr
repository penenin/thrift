require "./spec_helper"

describe Thrift::BinaryProtocol do

  it "should define the proper VERSION_1, VERSION_MASK AND TYPE_MASK" do
    Thrift::BinaryProtocol::VERSION_MASK.should eq 0xffff0000
    Thrift::BinaryProtocol::VERSION_1.should eq 0x8001
    Thrift::BinaryProtocol::TYPE_MASK.should eq 0x000000ff
  end

  it "should make strict_read readable" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.strict_read.should eq true
  end

  it "should make strict_write readable" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.strict_write.should eq true
  end

  it "should write the message header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_message_begin("testMessage", Thrift::MessageTypes::CALL, 17)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x80, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0B, 0x74, 0x65, 0x73, 0x74, 0x4D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x00, 0x00, 0x00, 0x11]
  end

  it "should write the message header without version when writes are not strict" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans, true, false)
    prot.write_message_begin("testMessage", Thrift::MessageTypes::CALL, 17)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x00, 0x00, 0x00, 0x0B, 0x74, 0x65, 0x73, 0x74, 0x4D, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x01, 0x00, 0x00, 0x00, 0x11]
  end

  it "should write the field header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_field_begin("foo", Thrift::Types::DOUBLE, 3)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x04, 0x00, 0x03]
  end

  it "should write the STOP field" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_field_stop
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x00]
  end

  it "should write the map header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_map_begin(Thrift::Types::STRING, Thrift::Types::LIST, 17)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x0B, 0x0F, 0x00, 0x00, 0x00, 0x11]
  end

  it "should write the list header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_list_begin(Thrift::Types::I16, 42)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x06, 0x00, 0x00, 0x00, 0x2A]
  end

  it "should write the set header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_set_begin(Thrift::Types::I16, 42)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x06, 0x00, 0x00, 0x00, 0x2A]
  end

  it "should write a bool" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_bool(true)
    prot.write_bool(false)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x01, 0x00]
  end

  it "should write a byte" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_byte(127)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x7F]
  end

  it "should write a i16" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_i16(32000_i16)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x7D, 0x00]
  end

  it "should write a i32" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_i32(1000000)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x00, 0x0F, 0x42, 0x40]
  end

  it "should write a i64" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_i64(5000000000)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x00, 0x00, 0x00, 0x01, 0x2A, 0x05, 0xF2, 0x00]
  end

  it "should write a double" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_double(0.1)
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x3F, 0xB9, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9A]
  end

  it "should write a string" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_string("abc")
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x00, 0x00, 0x00, 0x03, 0x61, 0x62, 0x63]
  end

  it "should write a binary" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    prot.write_binary(Bytes[0, 1, 2, 3])
    trans.reset_buffer
    trans.read(trans.available).should eq Bytes[0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x02, 0x03]
  end
 
  it "should read a field header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x0B, 0x00, 0x03])
    trans.reset_buffer
    prot.read_field_begin.should eq ({nil, Thrift::Types::STRING, 3})
  end

  it "should read a stop field" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x00, 0x00, 0x00])
    trans.reset_buffer
    prot.read_field_begin.should eq ({nil, Thrift::Types::STOP, 0})
  end

  it "should read a map header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x04, 0x0A, 0x00, 0x00, 0x00, 0x11])
    trans.reset_buffer
    prot.read_map_begin.should eq ({Thrift::Types::DOUBLE, Thrift::Types::I64, 17})
  end

  it "should read a list header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x0B, 0x00, 0x00, 0x00, 0x11])
    trans.reset_buffer
    prot.read_list_begin.should eq ({Thrift::Types::STRING, 17})
  end

  it "should read a set header" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x0B, 0x00, 0x00, 0x00, 0x11])
    trans.reset_buffer
    prot.read_set_begin.should eq ({Thrift::Types::STRING, 17})
  end

  it "should read a bool" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x01, 0x00])
    trans.reset_buffer
    prot.read_bool.should eq true
    prot.read_bool.should eq false
  end

  it "should read a i16" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x7D, 0x00])
    trans.reset_buffer
    prot.read_i16.should eq 32000
  end

  it "should read a i32" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x00, 0x0F, 0x42, 0x40])
    trans.reset_buffer
    prot.read_i32.should eq 1000000
  end

  it "should read a i64" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x00, 0x00, 0x00, 0x01, 0x2A, 0x05, 0xF2, 0x00])
    trans.reset_buffer
    prot.read_i64.should eq 5000000000
  end

  it "should read a double" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x3F, 0xB9, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9A])
    trans.reset_buffer
    prot.read_double.should eq 1.0
  end

  it "should read a string" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x00, 0x00, 0x00, 0x03, 0x61, 0x62, 0x63])
    trans.reset_buffer
    prot.read_string.should eq "abc"
  end

  it "should read a binary string" do
    trans = Thrift::MemoryBufferTransport.new
    prot = Thrift::BinaryProtocol.new(trans)
    trans.write(Bytes[0x00, 0x00, 0x00, 0x04, 0x00, 0x01, 0x02, 0x03])
    trans.reset_buffer
    prot.read_binary.should eq Bytes[0, 1, 2, 3]
  end
end
