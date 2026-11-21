#include "app.h"
#include <cstring>
void App::parse_init(parsedata_t *data)
{
	memset(data,0,sizeof(*data));
	data->stacksize = 0;
	data->pos = 0;
	data->book = NULL;
	data->prefs = NULL;
	data->screen = 0;
	data->pen.x = ts->margin.left;
	data->pen.y = ts->margin.top;
	data->linebegan = false;
	data->bold = false;
	data->italic = false;
	strcpy((char*)data->buf,"");
	data->cachefile = NULL;
	data->buflen = 0;
	data->status = 0;
}

void App::parse_push(parsedata_t *data, context_t context)
{
	if(data->status>=2) return;
	if(data->stacksize>=sizeof(data->stack)/sizeof(data->stack[0])) {data->status=2;return;}
	data->stack[data->stacksize++] = context;
}

context_t App::parse_pop(parsedata_t *data)
{
	if(data->status>=2 || !data->stacksize) return TAG_NONE;
	if (data->stacksize) data->stacksize--;
	return data->stack[data->stacksize];
}

bool App::parse_in(parsedata_t *data, context_t context)
{
	if(data->status>=2) return false;
	u8 i;
	for (i=0;i<data->stacksize;i++)
	{
		if (data->stack[i] == context) return true;
	}
	return false;
}

bool parse_append(parsedata_t* data,u8 value) {
	if(data->status>=2) return false;
	if(data->buflen<0 || data->buflen>=PAGEBUFSIZE) {data->status=2;return false;}
	data->buf[data->buflen++]=value;
	return true;
}
