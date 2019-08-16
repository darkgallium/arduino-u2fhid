#include <stdio.h>
#include <hidapi/hidapi.h>

// compiles with gcc -lhidapi-hidraw -o hid_enumerator hid_enumerator.c

int main() {
	struct hid_device_info *current;
	current = hid_enumerate(0,0);

	for (; current != NULL; current = current->next) {
		printf("[%s] %ls\n", current->path, current->product_string);
	}

	return 0;
}
