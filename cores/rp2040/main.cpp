extern void setup();
extern void loop();

extern "C" {

int main() {
	setup();
	while (1) loop();
	return 0;
}

}
