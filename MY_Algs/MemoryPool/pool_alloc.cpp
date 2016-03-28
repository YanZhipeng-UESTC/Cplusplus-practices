#include "pool_alloc.h"
pool_alloc_base::Obj* pool_alloc_base::freelist[freelist_size];
char* pool_alloc_base::start_free = nullptr;
char* pool_alloc_base::end_free = nullptr;
size_t pool_alloc_base::heap_size = 0;
void* pool_alloc_base::refill(size_t aligned_bytes)
{
	size_t nobjs = 20;
	char* chunk = alloc_chunk(aligned_bytes, nobjs);
	if (nobjs == 1)
		return static_cast<void*>(chunk);
	void* result = static_cast<void*>(chunk);
	Obj* current_obj;
	Obj* next_obj;
	Obj** freelist = get_free_list(aligned_bytes);
	current_obj = reinterpret_cast<Obj*>(chunk+aligned_bytes);
	*freelist = current_obj;
	for (size_t i = 2; i != nobjs; ++i)
	{
		next_obj = reinterpret_cast<Obj*>(chunk + i*aligned_bytes);
		current_obj->next_freelist_link = next_obj;
		current_obj = next_obj;
	}
	current_obj->next_freelist_link = nullptr;
	return result;
}
char* pool_alloc_base::alloc_chunk(size_t aligned_bytes, size_t &nobjs)
{
	char* result;
	size_t total_bytes = nobjs*aligned_bytes;
	size_t bytes_left = end_free - start_free;
	if (bytes_left >= total_bytes)
	{
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else if (bytes_left >= aligned_bytes)
	{
		result = start_free;
		nobjs = bytes_left / aligned_bytes;
		start_free += nobjs*aligned_bytes;
		return result;
	}
	else
	{
		if (bytes_left != 0)
		{
			Obj** freelist = get_free_list(bytes_left);
			Obj* lastfree = reinterpret_cast<Obj*>(start_free);
			lastfree->next_freelist_link = *freelist;
			*freelist = lastfree;
			start_free = end_free; //can be delete
		}
		try{
			start_free = static_cast<char*>(::operator new(2 * total_bytes));
			//start_free = static_cast<char*>(operator new(2 * total_bytes));
		}
		catch (std::bad_alloc& b)
		{
			Obj** freelist_left;
			for (size_t i = aligned_bytes + align; i <= max_size; i += align)
			{
				freelist_left = get_free_list(i);
				if (*freelist_left)
				{
					Obj* biggerfree = *freelist_left;
					*freelist_left = biggerfree->next_freelist_link;
					start_free = reinterpret_cast<char*>(biggerfree);
					end_free = start_free + i;
					return alloc_chunk(aligned_bytes, nobjs);
				}
			}
			start_free = end_free = nullptr;
			throw b;
		}
		end_free = start_free + 2 * total_bytes;
		heap_size += 2 * total_bytes;
		return alloc_chunk(aligned_bytes, nobjs);
	}
}
