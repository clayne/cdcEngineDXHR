#include <cstdio>
#include "FileReceiver.h"
#include "HackFileSystem.h"

namespace cdc {

FileRequest *HackFile::createRequest(FileReceiver *receiver, const char *path, uint32_t offset) {
	return new HackFileRequest(fs, f, receiver, offset);
}

uint32_t HackFile::getSize() {
	fseek(f->f, 0, SEEK_END);
	size_t size = ftell(f->f);
	fseek(f->f, 0, SEEK_SET);
	return (uint32_t) size;
}


void HackFileRequest::incrRefCount() {
	refCount++;
}

void HackFileRequest::decrRefCount() {
	if (--refCount == 0)
		delete this;
}

void HackFileRequest::setCompressedSize(uint32_t) {
	// empty
}

void HackFileRequest::setReadAmount(uint32_t size) {
	this->size = size;
}

uint32_t HackFileRequest::getCompletionStatus() {
	return completionStatus;	
}

void HackFileRequest::submit(uint8_t arg) {
	fs->requests.push_back(this);
}



FileRequest *HackFileSystem::createRequest(FileReceiver *receiver, const char *path, uint32_t offset) {
	FILE *f = fopen(path, "rb");
	HackFile hf(this, std::make_shared<FileWrapper>(f));
	uint32_t size = hf.getSize();
	auto *req = hf.createRequest(receiver, path, offset);
	req->setReadAmount(size);
	return req;
	// return new HackFileRequest(this, std::make_shared<FileWrapper>(f), receiver, offset);
}

File *HackFileSystem::createFile(const char *path) {
	FILE *f = fopen(path, "rb");
	return new HackFile(this, std::make_shared<FileWrapper>(f));
}

uint32_t HackFileSystem::getSize(const char *path) {
	FILE *f = fopen(path, "rb");
	HackFile hf(this, std::make_shared<FileWrapper>(f));
	return hf.getSize();
}

bool HackFileSystem::hasRequests() {
	return !requests.empty();
}

void HackFileSystem::processRequest() {
	if (requests.empty())
		return;
	HackFileRequest *req = requests.front();
	requests.erase(requests.begin());

	printf("processing request file %p, %d+%d -> %p\n", req->f->f, req->offset, req->size, req->receiver);
	char *buffer = new char[req->size];
	fseek(req->f->f, req->offset, SEEK_SET);
	fread(buffer, req->size, 1, req->f->f);
	req->receiver->process(req, buffer, req->size, 0);
	req->receiver->requestComplete(req);
	req->completionStatus = 3;

	delete[] buffer;
	req->decrRefCount();
}

}