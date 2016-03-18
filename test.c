#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>

static void countData(xmlNode *firstNode);
static void countYSets(xmlNode *firstNode);
static void getValues(xmlNode *firstNode);
static void printSpace(int times);
static void printHelpMessage();
static void CreatePieChart(xmlNode *root);
static void CreateBarChart(xmlNode *root);
static void CreateLineChart(xmlNode *root);
static int compare(const void * a,const void * b);

#define PI 3.14159265359

struct Canvas{
  char *length, *width, *backcolor;
};

struct Axis{
  char *name, *forecolor;
};

struct Set{
  char *unit, *name, *fillcolor;
  bool showValue;
  char **values;
};

char *title;
struct Canvas canvas;
struct Axis xaxis;
struct Axis yaxis;
struct Set xset;
struct Set ysets[10];
int ysetCount, dataCount;
char *colors[] = {"#FFFF00", "#5F5E5D", "#F2EEEA", "#B4B2AE", "#E24A37", "#231F20",
          "#F15E00", "#FDFF3A", "#F19C28", "#A12E33"};
char buffer[50];

int main(int argc, char *argv[]) {
  int i;
  char * fileName = NULL;
  char * outputName = NULL;
  char * validationName = NULL;
  char * type = NULL;
  bool wantHelp = 0;
  ysetCount = 0;
  dataCount = 0;

  for(i=0; i<argc; i++){
    if(i+1!=argc && !strcmp(argv[i], "-i")){
      fileName = argv[i+1];
    }else if(i+1!=argc && !strcmp(argv[i], "-o")){
      outputName = argv[i+1];
    }else if(i+1!=argc && !strcmp(argv[i], "-v")){
      validationName = argv[i+1];
    }else if(i+1!=argc && !strcmp(argv[i], "-t")){
      type = argv[i+1];
    }else if(!strcmp(argv[i], "-h")){
      wantHelp = 1;
      break;
    }
  }
  if(wantHelp){
    printHelpMessage();
    return 0;
  }
  if(fileName == NULL){
    printf("Main Message: There is no input file. \n\n");
    printHelpMessage();
    return 0;
  }else if(validationName == NULL){
    printf("Main Message: There is no validation file. \n\n");
    printHelpMessage();
    return 0;
  }else if(!(!strcmp(type, "line") || !strcmp(type, "pie") || !strcmp(type, "bar"))){
    printf("Main Message: Invalid typbuffer. \n\n");
    printHelpMessage();
    return 0;
  }

  xmlDoc *doc = xmlReadFile(fileName, NULL, 0);
  if(doc == NULL){
    printf("Main Message: Document not found. \n\n");
    printHelpMessage();
    return 0;
  }
  xmlNode *root = xmlDocGetRootElement(doc);

  xmlLineNumbersDefault(1);
  xmlSchemaParserCtxtPtr context = xmlSchemaNewParserCtxt(validationName);
  xmlSchemaPtr schema = xmlSchemaParse(context);
  xmlSchemaValidCtxtPtr validContext = xmlSchemaNewValidCtxt(schema);
  int validRes = xmlSchemaValidateDoc(validContext, doc);
  if(validRes != 0){
    printf("Main Message: XML error. \n\n");
    printHelpMessage();
    xmlCleanupParser();
    xmlFreeDoc(doc);
    xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    return 0;
  }
  countData(root->children);
  countYSets(root->children);

  getValues(root->children);

  xmlDocPtr newDoc  = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr newRoot = xmlNewNode(NULL, BAD_CAST "svg");
  sprintf(buffer, "%d", atoi(canvas.width)*2);
  xmlNewProp(newRoot, BAD_CAST "width", BAD_CAST buffer);
  sprintf(buffer, "%d", atoi(canvas.length)*ysetCount);
  xmlNewProp(newRoot, BAD_CAST "height", BAD_CAST buffer);
  xmlNewProp(newRoot, BAD_CAST "xmlns", BAD_CAST "http://www.w3.org/2000/svg");
  //HEX CHECK?
  sprintf(buffer, "background-color: #%s", canvas.backcolor);
  xmlNewProp(newRoot, BAD_CAST "style", BAD_CAST buffer);
  xmlDocSetRootElement(newDoc, newRoot);

  if(!strcmp(type, "pie")){
    CreatePieChart(newRoot);
  }else if(!strcmp(type, "bar")){
    CreateBarChart(newRoot);
  }else if(!strcmp(type, "line")){
    CreateLineChart(newRoot);
  }

  htmlSaveFileEnc(outputName, newDoc, "UTF-­8", 1);

  xmlCleanupParser();
  xmlFreeDoc(doc);
  xmlMemoryDump();
  xmlSchemaFree(schema);
  xmlSchemaCleanupTypes();
  return 0;
}

static int compare(const void * a,const void * b){
    return(*(int*)a - *(int*)b );
}

static void countData(xmlNode *firstNode){
  xmlNode *curNode = NULL;
  xmlNode *childNode = NULL;
  for(curNode = firstNode; curNode; curNode = curNode->next){
    if(curNode->type == XML_ELEMENT_NODE && (!strcmp(curNode->name, "Xset") || !strcmp(curNode->name, "Yset"))){
      for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(childNode->type == XML_ELEMENT_NODE){
            dataCount++;
          }
      }
      return;
    }
  }
}

static void countYSets(xmlNode *firstNode){
  xmlNode *curNode = NULL;
  for(curNode = firstNode; curNode; curNode = curNode->next){
    if(curNode->type == XML_ELEMENT_NODE && !strcmp(curNode->name, "Yset")){
      ysetCount++;
    }
  }
}

static void getValues(xmlNode *firstNode){
  xmlNode * curNode = NULL;
  xmlNode * childNode = NULL;
  xmlAttr * childAttr = NULL;
  int ysetIndex = 0;

  for(curNode = firstNode; curNode; curNode = curNode->next){
    if(curNode->type == XML_ELEMENT_NODE){
      if(!strcmp(curNode->name, "charttitle")){
          title = (char*) malloc(sizeof(char)*100);
          title = curNode->children->content;
      }else if(!strcmp(curNode->name, "canvas")){
          for(childNode = curNode->children; childNode; childNode = childNode->next){
            if(!strcmp(childNode->name, "length")){
                canvas.length = (char*) malloc(sizeof(char)*100);
                canvas.length = childNode->children->content;
            }else if(!strcmp(childNode->name, "width")){
                canvas.width = (char*) malloc(sizeof(char)*100);
                canvas.width = childNode->children->content;
            }else if(!strcmp(childNode->name, "backcolor")){
                canvas.backcolor = (char*) malloc(sizeof(char)*100);
                canvas.backcolor = childNode->children->content;
            }
          }
      }else if(!strcmp(curNode->name, "Xaxis")){
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "name")){
              xaxis.name = (char*) malloc(sizeof(char)*100);
              xaxis.name = childNode->children->content;
          }else if(!strcmp(childNode->name, "forecolor")){
              xaxis.forecolor = (char*) malloc(sizeof(char)*100);
              xaxis.forecolor = childNode->children->content;
          }
        }
      }else if(!strcmp(curNode->name, "Yaxis")){
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "name")){
              yaxis.name = (char*) malloc(sizeof(char)*100);
              yaxis.name = childNode->children->content;
          }else if(!strcmp(childNode->name, "forecolor")){
              xaxis.forecolor = (char*) malloc(sizeof(char)*100);
              yaxis.forecolor = childNode->children->content;
          }
        }
      }else if(!strcmp(curNode->name, "Xset")){
        xset.values = (char**) malloc((1+dataCount)*sizeof(char*));
        int i;
        for(i=0; i<dataCount; i++){
          xset.values[i] = (char*) malloc(sizeof(char)*100);
        }
        xset.unit = (char*) malloc(sizeof(char)*100);
        xset.fillcolor = (char*) malloc(sizeof(char)*100);
        xset.name = (char*) malloc(sizeof(char)*100);
        xset.showValue = 1;

        int valueIndex = 0;
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "xdata")){
            strcpy(xset.values[valueIndex], childNode->children->content);
            valueIndex++;
          }
        }
      }else if(!strcmp(curNode->name, "Yset")){
        ysets[ysetIndex].values = (char**) malloc((1+dataCount)*sizeof(char*));
        int i;
        for(i=0; i<dataCount; i++){
          ysets[ysetIndex].values[i] = (char*) malloc(sizeof(char)*100);
        }
        xset.unit = (char*) malloc(sizeof(char)*100);
        xset.fillcolor = (char*) malloc(sizeof(char)*100);
        xset.name = (char*) malloc(sizeof(char)*100);
        xset.showValue = 1;

        int valueIndex = 0;
        for(childNode = curNode->children; childNode; childNode = childNode->next){
          if(!strcmp(childNode->name, "ydata")){
            strcpy(ysets[ysetIndex].values[valueIndex], childNode->children->content);
            valueIndex++;
          }
        }

        for(childAttr = curNode->properties; childAttr; childAttr = childAttr->next){
          if(!strcmp(childAttr->name, "unit")){
            ysets[ysetIndex].unit = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "fillcolor")){
            ysets[ysetIndex].fillcolor = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "name")){
            ysets[ysetIndex].name = childAttr->children->content;
          }else if(!strcmp(childAttr->name, "showvalue")){
            if(!strcmp(childAttr->children->content, "yes")){
              ysets[ysetIndex].showValue = 1;
            }else{
              ysets[ysetIndex].showValue = 0;
            }
          }
        }

        ysetIndex++;
      }
    }
  }
}

static void printSpace(int times){
  int i;
  for(i=0; i<times; i++){
      printf(" ");
  }
}

static void printHelpMessage(){
  printf("NAME\n");
  printSpace(5);
  printf("chartgen - Writing SVG file from XML file\n\n");
  printSpace(5);
  printf("chartgen [-i <input fileName>]\n");
  printSpace(5);
  printf("         [-o <output fileName>]\n");
  printSpace(5);
  printf("         [-v <validation fileName>]\n");
  printSpace(5);
  printf("         [-t <type>]\n");
  printSpace(5);
  printf("         [-h]\n\n");
  printf("DESCRIPTION\n");
  printSpace(5);
  printf("         -i");
  printSpace(5);
  printf("Input file name which in XML format.\n");
  printSpace(5);
  printf("         -o");
  printSpace(5);
  printf("Output file name which written in SVG format.\n");
  printSpace(5);
  printf("         -v");
  printSpace(5);
  printf("Validate file name which checks XML is in correct format.\n");
  printSpace(5);
  printf("         -t");
  printSpace(5);
  printf("Type must be 'line', 'pie' or 'bar'\n");
  printSpace(5);
  printf("         -h");
  printSpace(5);
  printf("Writing help message.\n");
}

static void CreatePieChart(xmlNode *root){
  xmlNodePtr newNode;

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST title);
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "10");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "30");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");
  int i, j;
  char number[36];

  // MATH
  for(i=0; i<ysetCount; i++){
    char **strValues = ysets[i].values;
    int intValues[dataCount];
    int sum = 0;
    for(j=0; j<dataCount; j++){
      intValues[j] = atoi(strValues[j]);
      sum += intValues[j];
    }
    int pieValues[dataCount+1];
    pieValues[0] = 0;
    for(j=1; j<=dataCount; j++){
      pieValues[j] = 360*intValues[j-1]/sum;
    }
    for(j=1; j<=dataCount; j++){
      pieValues[j] += pieValues[j-1];
    }
    pieValues[dataCount] = 360;
    bool showValues = ysets[i].showValue;
    for(j=1; j<=dataCount; j++){
      if(showValues){
        newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST ysets[i].values[j-1]);
        xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "100");
        sprintf(number, "%d", 45+j*20+150*i);
        xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
        xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "black");
      }

      newNode = xmlNewChild(root, NULL, BAD_CAST "rect", NULL);
      xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "150");
      sprintf(number, "%d", 30+j*20+150*i);
      xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
      xmlNewProp(newNode, BAD_CAST "width", BAD_CAST "20");
      xmlNewProp(newNode, BAD_CAST "height", BAD_CAST "20");
      xmlNewProp(newNode, BAD_CAST "stroke", BAD_CAST "black");
      xmlNewProp(newNode, BAD_CAST "stroke-width", BAD_CAST "1");
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST colors[j-1]);

      //sprintf(number, "%dto%d-%s", pieValues[j-1], pieValues[j], xset.values[j-1]);
      newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST xset.values[j-1]);
      xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "180");
      sprintf(number, "%d", 45+j*20+150*i);
      xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");

      int a = pieValues[j-1];
      int b = pieValues[j];
      int mx = 50;
      int my = 100 + i*150;
      int x1 = mx + 30*cos(PI*a/180);
      int y1 = my + 30*sin(PI*a/180);
      int x2 = mx + 30*cos(PI*b/180);
      int y2 = my + 30*sin(PI*b/180);

      char path[255];
      sprintf(path, "M%d,%d  L%d, %d A30,30 0 0,1 %d,%d z", mx, my, x1, y1, x2, y2);

      xmlNodePtr newNode;
      newNode = xmlNewChild(root, NULL, BAD_CAST "path", NULL);
      xmlNewProp(newNode, BAD_CAST "d", BAD_CAST path);
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST colors[j-1]);

      newNode = xmlNewChild(root, NULL, BAD_CAST "text", ysets[i].name);
      sprintf(number, "%d", 150+i*150);
      xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "20");
      xmlNewProp(newNode, BAD_CAST "y", BAD_CAST number);
      xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "white");
    }
  }
}

static void CreateBarChart(xmlNode *root){
  xmlNodePtr nodeRect = NULL, nodeTspan = NULL, nodeLine = NULL,nodeG = NULL, nodeText = NULL;
  int barLength = (int)atoi(canvas.length) * 6 / 8 ;
	int xPosition;
	int yPosition;
	char spaceX[10], sizeA[10],spaceY[10],_spaceX[10];
  char buffer[255];
	yPosition = barLength /12;
	xPosition = (int)atoi(canvas.width) /2;
	int size = barLength*11/150;
	sprintf(sizeA, "%d", size);
	sprintf(spaceX, "%d", xPosition);
	sprintf(spaceY, "%d", yPosition);
  sprintf(buffer, "#%s", canvas.backcolor);

  //arka plan
  	nodeRect = xmlNewChild(root,NULL, BAD_CAST "rect", NULL);
  	xmlNewProp(nodeRect,BAD_CAST"width",BAD_CAST canvas.length);
  	xmlNewProp(nodeRect,BAD_CAST"height",BAD_CAST canvas.width);
  	xmlNewProp(nodeRect,BAD_CAST"fill",BAD_CAST buffer);

    //Başlık yazımı
    	nodeText = xmlNewChild(root,NULL, BAD_CAST "text",BAD_CAST title);
    	xmlNewProp(nodeText,BAD_CAST"x",BAD_CAST spaceX);
    	xmlNewProp(nodeText,BAD_CAST"y",BAD_CAST spaceY);
    	xmlNewProp(nodeText,BAD_CAST"text-anchor",BAD_CAST "middle");
    	xmlNewProp(nodeText,BAD_CAST"font-size",BAD_CAST sizeA);
    	xmlNewProp(nodeText,BAD_CAST"font-weight",BAD_CAST "bold");
    	xmlNewProp(nodeText,BAD_CAST"fill",BAD_CAST "#260A39");

    //eksen isimleri
    	yPosition = (barLength / 12);
    	xPosition = barLength / 6;
    	size = barLength*6/150;
    	sprintf(sizeA, "%d", size);
    	sprintf(spaceX, "%d", xPosition);
    	sprintf(spaceY, "%d", yPosition);

    	nodeText = xmlNewChild(root,NULL, BAD_CAST "text",BAD_CAST yaxis.name);
    	xmlNewProp(nodeText,BAD_CAST"x",BAD_CAST spaceX);
    	xmlNewProp(nodeText,BAD_CAST"y",BAD_CAST spaceY);
    	xmlNewProp(nodeText,BAD_CAST"text-anchor",BAD_CAST "middle");
    	xmlNewProp(nodeText,BAD_CAST"font-size",BAD_CAST sizeA);
    	xmlNewProp(nodeText,BAD_CAST"font-weight",BAD_CAST "bold");

    	yPosition =  ((int)atoi(canvas.length) * 7 / 8);
    	xPosition = ((int)atoi(canvas.length) * 15 / 16);
    	sprintf(spaceX, "%d", xPosition);
    	sprintf(spaceY, "%d", yPosition);

    	nodeText = xmlNewChild(root,NULL, BAD_CAST "text",BAD_CAST xaxis.name);
    	xmlNewProp(nodeText,BAD_CAST"x",BAD_CAST spaceX);
    	xmlNewProp(nodeText,BAD_CAST"y",BAD_CAST spaceY);
    	xmlNewProp(nodeText,BAD_CAST"text-anchor",BAD_CAST "middle");
    	xmlNewProp(nodeText,BAD_CAST"font-size",BAD_CAST sizeA);
    	xmlNewProp(nodeText,BAD_CAST"font-weight",BAD_CAST "bold");

    	nodeG = xmlNewChild(root, NULL, BAD_CAST "g",NULL);
    	xmlNewProp(nodeG,BAD_CAST"stroke-width",BAD_CAST "1.5");

      //---------------------------------  BarsDraw  ------------------------------------------------
      	int i;
      	int j;
      	int count = 0;
      	int temp = 0;
        int numberOfcity = ysetCount;
        int numberOfmonth = dataCount;
        int numberOfSale = dataCount;

      	int widthAllBar = barLength / numberOfmonth / (numberOfcity+1);

      	int salesArray[numberOfcity*numberOfSale];
      	int index=0;
      	for(i=0; i<numberOfcity; i++){
      		for(j=0; j<numberOfSale;j++){
      			salesArray[index]=(int)atoi(ysets[i].values[j]);
      			index++;
      		}
      	}
      	int sizeofArray=0;
      	sizeofArray=sizeof(salesArray)/sizeof(salesArray[0]);
      	qsort(salesArray,sizeofArray,sizeof(int),compare);

      	for (j = 0; j < numberOfcity; j++)
      	{
          sprintf(buffer, "#%s", ysets[j].fillcolor);

      		count= count + barLength/6; //x eksenindeki artış için
      		temp++;

      		for (i = 0; i <numberOfSale; i++)
      		{
      			int valueOfsale = (int)atoi(ysets[j].values[i]); //sale değerleri
      			int spaceFromRight = barLength / 6;
      			int lenghtOfRight = barLength + spaceFromRight; // toplam y eksen uzunluğu
      			int valueOfsales = (barLength * valueOfsale) / salesArray[index-1]; // barın y eksenindeki uzunluğu
      			int y = lenghtOfRight - valueOfsales; //barın yukardan kalan kısmı
      			xPosition = count;

      			char bar[12], spaceY[12], spaceW[12];
      			sprintf(bar, "%d", valueOfsales);
      			sprintf(spaceY, "%d", y);
      			sprintf(spaceX, "%d", xPosition);
      			sprintf(spaceW, "%d", widthAllBar);

      			count += widthAllBar *  (numberOfcity+1);

      			nodeRect = xmlNewChild(root, NULL, BAD_CAST "rect", NULL);
      			xmlNewProp(nodeRect, BAD_CAST"x", BAD_CAST spaceX);
      			xmlNewProp(nodeRect, BAD_CAST"y", BAD_CAST spaceY);
      			xmlNewProp(nodeRect, BAD_CAST"width", BAD_CAST spaceW);
      			xmlNewProp(nodeRect, BAD_CAST"height", BAD_CAST bar);
      			xmlNewProp(nodeRect, BAD_CAST"fill", BAD_CAST buffer);
      			xmlNewProp(nodeRect, BAD_CAST"stroke", BAD_CAST "black");

      			if(ysets[j].showValue){
      				char value[12];
      				sprintf(spaceY, "%d", y-((int)atoi(canvas.length)/30));
      				sprintf(spaceW,"%d" ,widthAllBar / 2);
      				sprintf(value, "%d", valueOfsale);

      				nodeTspan = xmlNewChild(nodeText, NULL, BAD_CAST "tspan", BAD_CAST value);
      				xmlNewProp(nodeTspan, BAD_CAST"x", BAD_CAST spaceX);
      				xmlNewProp(nodeTspan, BAD_CAST"y", BAD_CAST spaceY);
      				xmlNewProp(nodeTspan, BAD_CAST"writing-mode", BAD_CAST "tb");
      				xmlNewProp(nodeTspan, BAD_CAST"font-size", BAD_CAST spaceW);
      			}
      		}
      		count = widthAllBar * temp;
      	}

        //----------------------square-------------------------------
      	xPosition = barLength * 16 / 15 ;
      	yPosition = (barLength * 5) / 150;
      	size = barLength*6.25/150;

      	sprintf(spaceX, "%d", xPosition);
      	sprintf(spaceY, "%d", yPosition);
      	sprintf(sizeA, "%d", size);

      	for (i = 0; i < numberOfcity; i++)
      	{
      		nodeRect = xmlNewChild(nodeG, NULL, BAD_CAST "rect", NULL);
      		xmlNewProp(nodeRect, BAD_CAST"x", BAD_CAST spaceX);
      		xmlNewProp(nodeRect, BAD_CAST"y", BAD_CAST spaceY);
      		xmlNewProp(nodeRect, BAD_CAST"width", BAD_CAST sizeA);
      		xmlNewProp(nodeRect, BAD_CAST"height", BAD_CAST sizeA);
            sprintf(buffer, "#%s", ysets[i].fillcolor);
      		xmlNewProp(nodeRect, BAD_CAST"fill", BAD_CAST buffer);
      		xmlNewProp(nodeRect, BAD_CAST"stroke", BAD_CAST "black");

      		yPosition = yPosition + barLength * 7/ 150;
      		sprintf(spaceY, "%d", yPosition);
      	}

        //--------------------------------- MonthsWrite  ------------------------------------------------

        	size = barLength * 5 / 150;
        	sprintf(sizeA, "%d", size);
        	nodeG = xmlNewChild(root, NULL, BAD_CAST "g", NULL);
        	xmlNewProp(nodeG, BAD_CAST"font-size", BAD_CAST sizeA);

        	nodeText = xmlNewChild(nodeG, NULL, BAD_CAST "text", NULL);

        	count = 0;
        	yPosition = barLength*177/150;
        	sprintf(spaceY, "%d", yPosition);


        	for(i = 0; i <numberOfmonth; i++){

        		xPosition = barLength * ((barLength / 6 ) + count) / barLength;
        	    sprintf(spaceX, "%d", xPosition);

        		nodeTspan = xmlNewChild(nodeText, NULL, BAD_CAST "tspan", BAD_CAST xset.values[i]);
        		xmlNewProp(nodeTspan, BAD_CAST"x", BAD_CAST spaceX);
        		xmlNewProp(nodeTspan, BAD_CAST"y", BAD_CAST spaceY);
        		xmlNewProp(nodeTspan, BAD_CAST"writing-mode", BAD_CAST "tb");
        		xmlNewProp(nodeTspan, BAD_CAST"font-weight", BAD_CAST "bold");

        		count += widthAllBar *  (numberOfcity+1);

        	}

          //---------------------------------- SalesWrite   -------------------------------------------------
          	nodeText = xmlNewChild(nodeG,NULL, BAD_CAST "text", NULL);
          	int countOfy = barLength/6;
          	int gecici = salesArray[index-1] + (salesArray[index-1] / numberOfSale); //13125
          	int _xPosition = countOfy;
          	char geciciArr[10];
          	size = barLength*6/150;
          	sprintf(sizeA, "%d", size);

          	for(i=0; i<numberOfSale; i++){

          		xPosition = barLength * 15 / 150;
          		sprintf(spaceX, "%d", xPosition);
          		yPosition = countOfy;
          		sprintf(spaceY, "%d", yPosition);
          		gecici = gecici - (salesArray[index-1] / numberOfSale);
          		sprintf(geciciArr, "%d", gecici);

          		nodeTspan = xmlNewChild(nodeText, NULL, BAD_CAST "tspan", BAD_CAST geciciArr);
          		xmlNewProp(nodeTspan, BAD_CAST"x", BAD_CAST spaceX);
          		xmlNewProp(nodeTspan, BAD_CAST"y", BAD_CAST spaceY);
          		xmlNewProp(nodeTspan, BAD_CAST"text-anchor", BAD_CAST "middle");
          		xmlNewProp(nodeTspan, BAD_CAST"font-size", BAD_CAST sizeA);
          		xmlNewProp(nodeTspan, BAD_CAST"font-weight", BAD_CAST "bold");

          		countOfy += barLength / numberOfSale;

          		xPosition = (barLength / 6);
          		sprintf(spaceX, "%d", xPosition);
          		yPosition = (barLength * 175 / 150);
          		sprintf(spaceY, "%d", yPosition);

          		sprintf(_spaceX, "%d", _xPosition);

          		nodeLine = xmlNewChild(root,NULL,BAD_CAST"line", NULL);
          		xmlNewProp(nodeLine,BAD_CAST"x1",BAD_CAST spaceX);
          		xmlNewProp(nodeLine,BAD_CAST"y1",BAD_CAST _spaceX);
          		xmlNewProp(nodeLine,BAD_CAST"x2",BAD_CAST spaceY);
          		xmlNewProp(nodeLine,BAD_CAST"y2",BAD_CAST _spaceX);
          		xmlNewProp(nodeLine,BAD_CAST"style",BAD_CAST"stroke:rgb(0,0,0);stroke-dasharray:10,2 ;stroke-width:0.5");
          		xmlNewProp(nodeLine, BAD_CAST"font-size", BAD_CAST sizeA);
          		xmlNewProp(nodeLine, BAD_CAST"font-weight", BAD_CAST "bold");

          		_xPosition += (barLength / numberOfSale);

          	}

            //-------------------------------------- cityNameDraw------------------------------------------------

            	xPosition = barLength * 170 / 150;
            	sprintf(spaceX, "%d", xPosition);
            	yPosition = barLength * 10 / 150;
                sprintf(spaceY, "%d", yPosition);

            	for(i = 0; i < numberOfcity; i++){

            		nodeText = xmlNewChild(nodeG,NULL, BAD_CAST "text", BAD_CAST xset.values[i]);
            		xmlNewProp(nodeText, BAD_CAST"x", BAD_CAST spaceX);
            		xmlNewProp(nodeText, BAD_CAST"y", BAD_CAST spaceY);
            		xmlNewProp(nodeText, BAD_CAST"font-weight", BAD_CAST "bold");

            		yPosition = (barLength * 7 / 150) + yPosition;
               		sprintf(spaceY, "%d", yPosition);
            	}

            //----------------------------------------- LineDraw ----------------------------------

            	xPosition= barLength/6;
            	_xPosition = barLength + (barLength/6);
            	char arrX2[10],arrX1[10];
            	sprintf(spaceX,"%d",xPosition);
            	sprintf(_spaceX,"%d",_xPosition);

                for (i = 1; i < 3; i++)
                {
            		nodeLine = xmlNewChild(root,NULL, BAD_CAST "line",NULL);
            		xmlNewProp(nodeLine,BAD_CAST"x1",BAD_CAST ((i%2) ? spaceX :spaceX ));
            		xmlNewProp(nodeLine,BAD_CAST"y1",BAD_CAST ((i%2) ? _spaceX :spaceX ));
            		xmlNewProp(nodeLine,BAD_CAST"x2",BAD_CAST ((i%2) ? _spaceX :spaceX ));
            		xmlNewProp(nodeLine,BAD_CAST"y2",BAD_CAST ((i%2) ? _spaceX :_spaceX ));
            		xmlNewProp(nodeLine,BAD_CAST"stroke",BAD_CAST "black");
            		xmlNewProp(nodeLine,BAD_CAST"stroke-width",BAD_CAST "3");

            	}
}

static void CreateLineChart(xmlNode *root){
  xmlNodePtr newNode, lineNode;

  newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST title);
  xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "10");
  xmlNewProp(newNode, BAD_CAST "y", BAD_CAST "15");
  xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");

  int i, j;
  char allPoints[255];
  char point[25];

  int maxValue = 0;
  for(j = 0; j < ysetCount; j++){
    for(i = 0; i < dataCount; i++){
      int value = atoi(ysets[j].values[i]);
      if(value > maxValue){
        maxValue = value;
      }
    }
  }

//eksenleri çizdirme
  newNode = xmlNewChild(root, NULL, BAD_CAST "line",NULL);
  xmlNewProp(newNode, BAD_CAST "x1", BAD_CAST "80");
  xmlNewProp(newNode, BAD_CAST "x2", BAD_CAST "300");
  xmlNewProp(newNode, BAD_CAST "y1", BAD_CAST "200");
  xmlNewProp(newNode, BAD_CAST "y2", BAD_CAST "200");
  xmlNewProp(newNode, BAD_CAST "style", BAD_CAST "stroke:rgb(255,0,0);stroke-width:3");

  newNode = xmlNewChild(root, NULL, BAD_CAST "line",NULL);
  xmlNewProp(newNode, BAD_CAST "x1", BAD_CAST "80");
  xmlNewProp(newNode, BAD_CAST "x2", BAD_CAST "80");
  xmlNewProp(newNode, BAD_CAST "y1", BAD_CAST "20");
  xmlNewProp(newNode, BAD_CAST "y2", BAD_CAST "200");
  xmlNewProp(newNode, BAD_CAST "style", BAD_CAST "stroke:rgb(255,0,0);stroke-width:3");

// ay isimlerini yazdırma
  for(i = 1; i <= dataCount; i++){
    newNode = xmlNewChild(root, NULL, BAD_CAST "text", xset.values[i-1]);
    int y = -60 - (int) ((float)i*((float) 170/dataCount));
    sprintf(buffer, "%d", y);
    xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "220");
    xmlNewProp(newNode, BAD_CAST "y", BAD_CAST buffer);
    xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "white");
    xmlNewProp(newNode, BAD_CAST "transform", BAD_CAST "rotate(90 20,10)");
  }

  // yazıları yazdırma
  for(j = 1; j <= 5; j++){
    int value = j*maxValue/5;
    sprintf(buffer, "%d", value);
    newNode = xmlNewChild(root, NULL, BAD_CAST "text", buffer);
    xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "50");
    int y = 208 - (int) ((float)value*((float) 180/maxValue));
    sprintf(buffer, "%d", y);
    xmlNewProp(newNode, BAD_CAST "y", BAD_CAST buffer);
    xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "white");
    xmlNewProp(newNode, BAD_CAST "style", BAD_CAST "font-size:8px");

    newNode = xmlNewChild(root, NULL, BAD_CAST "line", NULL);
    xmlNewProp(newNode, BAD_CAST "x1", BAD_CAST "80");
    xmlNewProp(newNode, BAD_CAST "x2", BAD_CAST "300");
    xmlNewProp(newNode, BAD_CAST "y1", BAD_CAST buffer);
    xmlNewProp(newNode, BAD_CAST "y2", BAD_CAST buffer);
    xmlNewProp(newNode, BAD_CAST "style", BAD_CAST "stroke:rgb(100, 100, 100);stroke-width:1");
    xmlNewProp(newNode, BAD_CAST "stroke-dasharray", BAD_CAST "1, 2");
  }

  // legend
  for(j = 0; j < ysetCount; j++){
    char color[255];
    sprintf(color, "#%s", ysets[j].fillcolor);

    newNode = xmlNewChild(root, NULL, BAD_CAST "rect", NULL);
    xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "310");
    sprintf(buffer, "%d", 30+j*30);
    xmlNewProp(newNode, BAD_CAST "y", BAD_CAST buffer);
    xmlNewProp(newNode, BAD_CAST "width", BAD_CAST "20");
    xmlNewProp(newNode, BAD_CAST "height", BAD_CAST "20");
    xmlNewProp(newNode, BAD_CAST "stroke", BAD_CAST "black");
    xmlNewProp(newNode, BAD_CAST "stroke-width", BAD_CAST "1");
    xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST color);

    newNode = xmlNewChild(root, NULL, BAD_CAST "text", BAD_CAST ysets[j].name);
    xmlNewProp(newNode, BAD_CAST "x", BAD_CAST "335");
    sprintf(buffer, "%d", 47+j*30);
    xmlNewProp(newNode, BAD_CAST "y", BAD_CAST buffer);
    xmlNewProp(newNode, BAD_CAST "fill", BAD_CAST "red");
  }

 // çizgileri çizdirme
  for(j = 0; j < ysetCount; j++){
    newNode = xmlNewChild(root, NULL, BAD_CAST "polyline",NULL);
    // kordinatlar x1,y1,x2,y2.. şeklinde duruyo || Matematik kısmı
    int coordinates[dataCount*2+4];
    coordinates[0] = 80;
    coordinates[1] = 200;
    int x,y;
    for(i = 1; i <= dataCount; i++){
      int value = atoi(ysets[j].values[i-1]);
      x = 80 + (int) ((float)i*((float) 170/dataCount));
      coordinates[2*i] = x;
      y = 200 - (int) ((float)value*((float) 180/maxValue));
      coordinates[2*i+1] = y;
    }
    // çizgilere renk verme
    char color[255];
    sprintf(color, "#%s", ysets[j].fillcolor);
    sprintf(buffer, "fill:none;stroke:%s;stroke-width:3", color);
    // noktaları yazdırma kısmı
    sprintf(allPoints, "");
    xmlNewProp(newNode, BAD_CAST "style", BAD_CAST buffer);
    for(i = 0; i <= dataCount; i++){
      sprintf(point, " %d,%d", coordinates[2*i], coordinates[2*i+1]);
      strcat(allPoints, point);
    }
    xmlNewProp(newNode, BAD_CAST "points", BAD_CAST allPoints);
  }
}
