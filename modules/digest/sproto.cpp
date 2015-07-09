/*************************************************************************/
/*  sproto.cpp                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "sproto.h"
#include <memory.h>

extern "C" {
#include "sproto/sproto.h"
};

void Sproto::dump() {

	ERR_FAIL_COND(proto == NULL);
	sproto_dump(proto);
}

int Sproto::proto_tag(const String& p_type) {

	ERR_FAIL_COND_V(proto == NULL, -1);
	return sproto_prototag(proto, p_type.utf8().get_data());
}

String Sproto::proto_name(int p_tag) {

	ERR_FAIL_COND_V(proto == NULL, "");
	return sproto_protoname(proto, p_tag);
}

bool Sproto::proto_has(const String& p_type, Proto p_what) {

	ERR_FAIL_COND_V(proto == NULL, false);
	int tag = sproto_prototag(proto, p_type.utf8().get_data());
	ERR_FAIL_COND_V(tag == -1, false);
	struct sproto_type *st = sproto_protoquery(proto, tag, p_what);
	return st != NULL;
}

#define ENCODE_BUFFERSIZE 2050
#define ENCODE_MAXSIZE 0x1000000

void Sproto::_expand_buffer(int sz) {

	if(wbuffer.size() < sz)
		wbuffer.resize(sz);
}

ByteArray Sproto::pack(const ByteArray& p_stream) {

	//int sproto_pack(const void * src, int srcsz, void * buffer, int bufsz);
	int sz = p_stream.size();
	ByteArray::Read r = p_stream.read();
	const void *buffer = r.ptr();
	// the worst-case space overhead of packing is 2 bytes per 2 KiB of input (256 words = 2KiB).
	int maxsz = (sz + 2047) / 2048 * 2 + sz;
	_expand_buffer(maxsz);

	ByteArray::Write w = wbuffer.write();
	void *output = w.ptr();

	int bytes = sproto_pack(buffer, sz, output, maxsz);
	ERR_EXPLAIN("packing error, return size = " + String::num(bytes));
	ERR_FAIL_COND_V(bytes > maxsz, ByteArray());

	r = wbuffer.read();
	ByteArray result;
	result.resize(bytes);
	w = result.write();
	memcpy(w.ptr(), r.ptr(), bytes);
	return result;
}

ByteArray Sproto::unpack(const ByteArray& p_stream) {

	int sz = p_stream.size();
	ByteArray::Read r = p_stream.read();
	const void *buffer = r.ptr();
	ByteArray::Write w = wbuffer.write();
	void *output = w.ptr();
	int osz = wbuffer.size();
	int ret = sproto_unpack(buffer, sz, output, osz);
	ERR_EXPLAIN("Invalid unpack stream");
	ERR_FAIL_COND_V(ret < 0, ByteArray());
	if (ret > osz) {
		w = ByteArray::Write();
		_expand_buffer(ret);
		w = wbuffer.write();
		output = w.ptr();
		ret = sproto_unpack(buffer, sz, output, ret);
		ERR_EXPLAIN("Invalid unpack stream");
		ERR_FAIL_COND_V(ret < 0, ByteArray());
	}
	r = wbuffer.read();
	ByteArray result;
	result.resize(ret);
	w = result.write();
	memcpy(w.ptr(), r.ptr(), ret);
	return result;
}

void Sproto::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("dump"),&Sproto::dump);
	ObjectTypeDB::bind_method(_MD("get_default","type"),&Sproto::get_default);
	ObjectTypeDB::bind_method(_MD("encode","type","dict"),&Sproto::encode);
	ObjectTypeDB::bind_method(_MD("decode","type","stream","use_default"),&Sproto::decode,false);
	ObjectTypeDB::bind_method(_MD("pack","stream"),&Sproto::pack);
	ObjectTypeDB::bind_method(_MD("unpack","stream"),&Sproto::unpack);

	ObjectTypeDB::bind_method(_MD("proto_tag","type"),&Sproto::proto_tag);
	ObjectTypeDB::bind_method(_MD("proto_name","tag"),&Sproto::proto_name);
	ObjectTypeDB::bind_method(_MD("proto_has","type","what"),&Sproto::proto_has);
	ObjectTypeDB::bind_method(_MD("proto_get_default","type","what"),&Sproto::proto_get_default);
	ObjectTypeDB::bind_method(_MD("proto_encode","type","what","dict"),&Sproto::proto_encode);
	ObjectTypeDB::bind_method(_MD("proto_decode","type","what","stream","use_default"),&Sproto::proto_decode,false);

	BIND_CONSTANT(REQUEST);
	BIND_CONSTANT(RESPONSE);
}

Sproto::Sproto()
	: proto(NULL)
{
	wbuffer.resize(ENCODE_BUFFERSIZE);
}

Sproto::~Sproto() {

	if(proto != NULL)
		sproto_release(proto);
}
