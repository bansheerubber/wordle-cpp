target = wordle
cclibs = -lpthread
ccinclude = -Iinclude/robin-map/include
CC = g++
CPPFLAGS = -g -O2 -Wall -Wno-switch -Wno-class-memaccess -Wno-delete-incomplete -Wno-attributes -Bsymbolic -fPIC -fno-semantic-interposition --static -std=c++17 -ffast-math
soflags =
ldflags =

cpp_source = $(shell find src -type f -name "*.cc" ! -path "src/include*")
cpp_source_tmp = $(subst src, tmp, $(cpp_source))
cpp_source_without = $(subst src\/, , $(cpp_source))

cpp_headers = $(shell find src -type f -name "*.h" ! -path "src/include*") 
cpp_headers_tmp = $(subst src, tmp, $(cpp_headers))

cpp_objects = $(patsubst %.cc, %.o, $(cpp_source))
cpp_objects_tmp = $(patsubst %.cc, %.o, $(cpp_source_tmp))
cpp_objects_without = $(patsubst src\/, , $(cpp_source))

.PHONY: default clean

# force synchronization for preprocessor
default:
	@"$(MAKE)" preprocessor --no-print-directory
	@"$(MAKE)" dist/$(target) --no-print-directory

preprocessor:
	@echo -e "   PY      tools/preprocessor.py"
	@python3 tools/preprocessor.py

$(cpp_objects_tmp) : %.o : %.cc
	@mkdir -p $(dir $@)
	@echo -e "   CC      $<"
	@$(CC) $(CPPFLAGS) $(soflags) $(ccinclude) -c $< -o $@

dist/$(target): $(cpp_objects_tmp)
	@mkdir -p $(dir dist/$(target))
	@echo -e "   CC      $@"
	@$(CC) $(cpp_objects_tmp) -Wall $(ccinclude) $(cclibs) -o $@

clean:
	@echo -e "   RM      tmp"
	@rm -Rf tmp

	@echo -e "   RM      dist/$(target)"
	@rm -f dist/$(target)