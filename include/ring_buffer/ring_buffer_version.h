#ifndef RING_BUFFER_VERSION_H
#define RING_BUFFER_VERSION_H

const char *ring_buffer_get_version();
unsigned ring_buffer_get_version_major();
unsigned ring_buffer_get_version_minor();
unsigned ring_buffer_get_version_patch();
const char *ring_buffer_get_git_sha();
const char *ring_buffer_get_git_date();
const char *ring_buffer_get_git_subject();

#endif //RING_BUFFER_VERSION_H