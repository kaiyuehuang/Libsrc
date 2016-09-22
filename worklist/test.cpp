#include <stdio.h>
using namespace std;  
class Test  
{  
private:  
	int x,y;  
public:  
	void setX(int a);
	void setY(int b);  
	void getXY(int *px, int *py)  
	{  
		*px=x;    //提取x,y值  
		*py=y;  
	}  
};  

void Test::setX(int a)  
{  
	x=a;  
}  
void Test::setY(int b)  
{  
	y=b;  
}  
int main()  
{  
	int a,b;  
	Test p1;  
	p1.setX(128);  
	p1.setY(256);  
	p1.getXY(&a,&b);  //将 a=x, b=y  
	printf("a=%d,b=%d\n",a,b );
}