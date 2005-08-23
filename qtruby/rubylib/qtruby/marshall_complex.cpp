template <>
static void marshall_from_ruby<long long>(Marshall *m) 
{
	VALUE obj = *(m->var());
	m->item().s_voidp = new long long;
	*(long long *)m->item().s_voidp = ruby_to_primitive<long long>(obj);
	
	m->next();
	
	if(m->cleanup() && m->type().isConst()) {
		delete (long long int *) m->item().s_voidp;
	}	
}

template <>
static void marshall_from_ruby<unsigned long long>(Marshall *m) 
{
	VALUE obj = *(m->var());
	m->item().s_voidp = new unsigned long long;
	*(long long *)m->item().s_voidp = ruby_to_primitive<unsigned long long>(obj);

	m->next();
	
	if(m->cleanup() && m->type().isConst()) {
		delete (long long int *) m->item().s_voidp;
	}	
}

