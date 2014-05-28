
int fun(int c);

int main(){
	fun(10);

}

int fun(int c){
	int a,b;
	B1:
	a = 0;
	B2:
	b = a + 1;
	B3:
	c = c + b;
	B4:
	a = b * 2;
	B5:
	if(a< 100) goto B2;
	B6:
	return c;
}