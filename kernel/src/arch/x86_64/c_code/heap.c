#include"heap.h"

uint8_t *brk;
free_linked_list_entry_t *list_of_free_heads[13];

void kmem_init() {
	my_memset((uint8_t *) list_of_free_heads, 0x00, 13 * sizeof(free_linked_list_entry_t *));
	brk = (uint8_t *) (((uint64_t) 1) << 40);
	make_pt_valid(brk);
}

uint8_t *kmalloc(uint64_t size) {
	uint8_t *ret_val;
	uint8_t power_of_2;
	uint8_t *curr_pf, *prev_pf;
	int64_t i, j;

	if (size == 0) {
		return (uint8_t *) 0;
	}
	

	power_of_2 = log_2_roof(size + sizeof(allocated_block_header_t));

	if (power_of_2 < 12) {
		if (power_of_2 < 5) {
			power_of_2 = 5;
		}

		if (list_of_free_heads[power_of_2] == (free_linked_list_entry_t *) 0) {
			grow(power_of_2);
		}
		
		list_of_free_heads[power_of_2] -> addr -> size = size;

		ret_val = ((uint8_t *) (list_of_free_heads[power_of_2] -> addr)) + sizeof(allocated_block_header_t);
		list_of_free_heads[power_of_2] = list_of_free_heads[power_of_2] -> next;
	} else {
		prev_pf = (uint8_t *) 0;
		
		for(i = 0; i < div_4096(size + sizeof(allocated_block_header_t)); i++) {
			curr_pf = pfalloc();
			if (prev_pf == (uint8_t *) 0) {
				ret_val = curr_pf + sizeof(allocated_block_header_t);
				((allocated_block_header_t *) curr_pf) -> size = size;
				prev_pf = curr_pf;
			} else if (curr_pf - prev_pf != 4096) {
				pffree(curr_pf);
				for(j = i-1; j >= 0; j--) {
					pffree(ret_val - sizeof(allocated_block_header_t) + j * 4096);
				}
				i = -1;
				prev_pf = (uint8_t *) 0;
			} else {
				prev_pf = curr_pf;
			}
		}
	}

	return ret_val;
}

void kfree(uint8_t *ptr) {
	allocated_block_header_t *temp;
	allocated_block_header_t *block = (allocated_block_header_t *) (ptr - sizeof(allocated_block_header_t));
	uint8_t power_of_2 = log_2_roof(block -> size + sizeof(allocated_block_header_t));
	int64_t i;

	if (power_of_2 < 12) {
		if (power_of_2 < 5) {
			power_of_2 = 5;
		}

		block -> size = 0;
		list_of_free_heads[power_of_2] = create_new_entry(block, list_of_free_heads[power_of_2]);
	} else {
		for(i = div_4096(block -> size + sizeof(allocated_block_header_t))-1; i >= 0; i --) {
			pffree((uint8_t *) block + i * 4096);
		}
	}
}

void grow(uint16_t power_of_2) {
	uint8_t *old_brk = brk;
	brk += (1 << power_of_2) + sizeof(allocated_block_header_t);
	((allocated_block_header_t *) old_brk) -> size = 0;
	make_pt_valid(brk);

	list_of_free_heads[power_of_2] = create_new_entry((allocated_block_header_t *) (old_brk + sizeof(allocated_block_header_t)), (free_linked_list_entry_t *) 0);
}

free_linked_list_entry_t *create_new_entry(allocated_block_header_t *addr, free_linked_list_entry_t *next) {
	free_linked_list_entry_t *entry = (free_linked_list_entry_t *) brk;
	brk += sizeof(free_linked_list_entry_t);
	make_pt_valid(brk);
	entry -> addr = addr;
	entry -> next = next;
	return entry;
}



uint8_t log_2_roof(uint64_t i) {
	uint8_t ret = -1;
	uint8_t greater_than_power_of_2 = 0;

	while(i) {
		if (1 != i && (i % 2) == 1){
			greater_than_power_of_2 = 1;
		}

		i = i >> 1;
		ret += 1;
	}

	return ret + greater_than_power_of_2;
}

uint64_t div_4096(uint64_t value) {
	if ((value % 4096) == 0) {
		return value / 4096;
	}

	return (value / 4096) + 1;
}


