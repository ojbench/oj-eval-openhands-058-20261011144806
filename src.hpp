#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>

class BasicException {
protected:
    const char *message;

public:
    explicit BasicException(const char *_message) : message(_message) {
    }

    virtual const char *what() const {
        return message;
    }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const char *_message) : BasicException(_message) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const char *_message) : BasicException(_message) {}
};

struct Pokemon {
    char name[12];
    int id;
    std::vector<std::string> types;
    
    Pokemon() : id(0) {
        name[0] = '\0';
    }
    
    Pokemon(const char *pname, int pid, const char *ptypes, bool validate = true) : id(pid) {
        strncpy(name, pname, 11);
        name[11] = '\0';
        
        std::string types_str(ptypes);
        std::stringstream ss(types_str);
        std::string type;
        
        while (std::getline(ss, type, '#')) {
            types.push_back(type);
        }
        
        if (validate) {
            // This will be called from user operations, not file loading
        }
    }
};

class Pokedex {
private:
    std::map<int, Pokemon> pokemons;
    std::string filename;
    
    // Type effectiveness map: attacker -> defender -> multiplier
    std::map<std::string, std::map<std::string, float>> typeChart;
    
    void initTypeChart() {
        // Fire
        typeChart["fire"]["grass"] = 2.0f;
        typeChart["fire"]["ice"] = 2.0f;
        typeChart["fire"]["water"] = 0.5f;
        
        // Water
        typeChart["water"]["fire"] = 2.0f;
        typeChart["water"]["ground"] = 2.0f;
        typeChart["water"]["grass"] = 0.5f;
        
        // Grass
        typeChart["grass"]["water"] = 2.0f;
        typeChart["grass"]["electric"] = 2.0f;
        typeChart["grass"]["fire"] = 0.5f;
        typeChart["grass"]["ice"] = 0.5f;
        
        // Electric
        typeChart["electric"]["water"] = 2.0f;
        typeChart["electric"]["flying"] = 2.0f;
        typeChart["electric"]["ground"] = 0.0f;
        typeChart["electric"]["grass"] = 0.5f;
        
        // Ice
        typeChart["ice"]["grass"] = 2.0f;
        typeChart["ice"]["ground"] = 2.0f;
        typeChart["ice"]["flying"] = 2.0f;
        typeChart["ice"]["fire"] = 0.5f;
        
        // Ground
        typeChart["ground"]["fire"] = 2.0f;
        typeChart["ground"]["electric"] = 2.0f;
        typeChart["ground"]["flying"] = 0.0f;
        
        // Flying
        typeChart["flying"]["grass"] = 2.0f;
        typeChart["flying"]["electric"] = 2.0f;
        typeChart["flying"]["ground"] = 2.0f;
    }
    
    bool isValidName(const char *name) const {
        if (!name || strlen(name) == 0 || strlen(name) > 10) {
            return false;
        }
        
        for (int i = 0; name[i]; i++) {
            if (!((name[i] >= 'a' && name[i] <= 'z') || (name[i] >= 'A' && name[i] <= 'Z'))) {
                return false;
            }
        }
        return true;
    }
    
    bool isValidType(const std::string &type) const {
        const std::set<std::string> validTypes = {
            "fire", "water", "grass", "electric", "ice", "ground", "flying"
        };
        return validTypes.find(type) != validTypes.end();
    }
    
    void validateTypes(const char *types) const {
        std::string types_str(types);
        std::stringstream ss(types_str);
        std::string type;
        int count = 0;
        
        while (std::getline(ss, type, '#')) {
            count++;
            if (!isValidType(type)) {
                throw ArgumentException(("Argument Error: PM Type Invalid (" + type + ")").c_str());
            }
        }
        
        if (count < 1 || count > 7) {
            throw ArgumentException("Argument Error: PM Type Invalid (invalid count)");
        }
    }
    
    void loadFromFile() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return;
        }
        
        pokemons.clear();
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            std::stringstream ss(line);
            std::string name_str, id_str, types_str;
            
            if (std::getline(ss, name_str, ',') && 
                std::getline(ss, id_str, ',') && 
                std::getline(ss, types_str)) {
                
                // Remove "name=" prefix
                if (name_str.substr(0, 5) == "name=") {
                    name_str = name_str.substr(5);
                }
                
                // Remove "id=" prefix
                if (id_str.substr(0, 3) == "id=") {
                    id_str = id_str.substr(3);
                }
                
                // Remove "types=" prefix
                if (types_str.substr(0, 6) == "types=") {
                    types_str = types_str.substr(6);
                }
                
                try {
                    int id = std::stoi(id_str);
                    Pokemon pm(name_str.c_str(), id, types_str.c_str(), false);
                    pokemons[id] = pm;
                } catch (...) {
                    // Skip invalid entries during file loading
                    continue;
                }
            }
        }
        
        file.close();
    }
    
    void saveToFile() {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return;
        }
        
        for (const auto &pair : pokemons) {
            const Pokemon &pm = pair.second;
            file << "name=" << pm.name << ",id=" << pm.id << ",types=";
            
            for (size_t i = 0; i < pm.types.size(); i++) {
                if (i > 0) file << "#";
                file << pm.types[i];
            }
            file << std::endl;
        }
        
        file.close();
    }

public:
    explicit Pokedex(const char *_fileName) : filename(_fileName) {
        initTypeChart();
        loadFromFile();
    }

    ~Pokedex() {
        saveToFile();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        // Validate inputs
        if (!isValidName(name)) {
            throw ArgumentException(("Argument Error: PM Name Invalid (" + std::string(name) + ")").c_str());
        }
        
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        validateTypes(types);
        
        // Check if name already exists
        for (const auto &pair : pokemons) {
            if (strcmp(pair.second.name, name) == 0) {
                return false;
            }
        }
        
        // Check if id already exists
        if (pokemons.find(id) != pokemons.end()) {
            return false;
        }
        
        // Add pokemon
        Pokemon pm(name, id, types);
        pokemons[id] = pm;
        return true;
    }

    bool pokeDel(int id) {
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        auto it = pokemons.find(id);
        if (it != pokemons.end()) {
            pokemons.erase(it);
            return true;
        }
        return false;
    }

    std::string pokeFind(int id) const {
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        auto it = pokemons.find(id);
        if (it != pokemons.end()) {
            return std::string(it->second.name);
        }
        return "None";
    }

    std::string typeFind(const char *types) const {
        validateTypes(types);
        
        std::set<std::string> searchTypes;
        std::string types_str(types);
        std::stringstream ss(types_str);
        std::string type;
        
        while (std::getline(ss, type, '#')) {
            searchTypes.insert(type);
        }
        
        std::vector<Pokemon> results;
        
        for (const auto &pair : pokemons) {
            const Pokemon &pm = pair.second;
            bool match = false;
            
            for (const std::string &pmType : pm.types) {
                if (searchTypes.find(pmType) != searchTypes.end()) {
                    match = true;
                    break;
                }
            }
            
            if (match) {
                results.push_back(pm);
            }
        }
        
        if (results.empty()) {
            return "None";
        }
        
        std::sort(results.begin(), results.end(), 
                 [](const Pokemon &a, const Pokemon &b) { return a.id < b.id; });
        
        std::string result = std::to_string(results.size());
        for (const Pokemon &pm : results) {
            result += "\n" + std::string(pm.name);
        }
        
        return result;
    }

    float attack(const char *type, int id) const {
        if (!isValidType(type)) {
            throw ArgumentException(("Argument Error: PM Type Invalid (" + std::string(type) + ")").c_str());
        }
        
        if (id <= 0) {
            throw ArgumentException(("Argument Error: PM ID Invalid (" + std::to_string(id) + ")").c_str());
        }
        
        auto it = pokemons.find(id);
        if (it == pokemons.end()) {
            return -1.0f;
        }
        
        const Pokemon &defender = it->second;
        float totalDamage = 1.0f;
        
        for (const std::string &defType : defender.types) {
            auto attackerIt = typeChart.find(type);
            if (attackerIt != typeChart.end()) {
                auto defenderIt = attackerIt->second.find(defType);
                if (defenderIt != attackerIt->second.end()) {
                    totalDamage *= defenderIt->second;
                }
            }
        }
        
        return totalDamage;
    }

    int catchTry() const {
        if (pokemons.empty()) {
            return 0;
        }
        
        // Start with the pokemon with smallest id
        auto it = pokemons.begin();
        Pokemon starter = it->second;
        
        std::set<int> owned;
        owned.insert(starter.id);
        
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto &pair : pokemons) {
                int targetId = pair.first;
                const Pokemon &target = pair.second;
                
                if (owned.find(targetId) != owned.end()) {
                    continue; // Already owned
                }
                
                // Check if any owned pokemon can catch this target
                bool canCatch = false;
                for (int ownedId : owned) {
                    const Pokemon &owner = pokemons.at(ownedId);
                    
                    // Try each type of the owned pokemon
                    for (const std::string &attackType : owner.types) {
                        float damage = attack(attackType.c_str(), targetId);
                        if (damage >= 2.0f) {
                            canCatch = true;
                            break;
                        }
                    }
                    
                    if (canCatch) break;
                }
                
                if (canCatch) {
                    owned.insert(targetId);
                    changed = true;
                }
            }
        }
        
        return owned.size();
    }

    struct iterator {
    private:
        std::map<int, Pokemon>::iterator map_it;
        std::map<int, Pokemon> *pokemons_ptr;
        
    public:
        iterator() : pokemons_ptr(nullptr) {}
        
        iterator(std::map<int, Pokemon>::iterator it, std::map<int, Pokemon> *ptr) 
            : map_it(it), pokemons_ptr(ptr) {}
        
        iterator &operator++() {
            if (pokemons_ptr && map_it != pokemons_ptr->end()) {
                ++map_it;
            }
            return *this;
        }
        
        iterator &operator--() {
            if (pokemons_ptr && map_it != pokemons_ptr->begin()) {
                --map_it;
            }
            return *this;
        }
        
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        
        iterator & operator = (const iterator &rhs) {
            map_it = rhs.map_it;
            pokemons_ptr = rhs.pokemons_ptr;
            return *this;
        }
        
        bool operator == (const iterator &rhs) const {
            return map_it == rhs.map_it;
        }
        
        bool operator != (const iterator &rhs) const {
            return map_it != rhs.map_it;
        }
        
        Pokemon & operator*() const {
            if (pokemons_ptr && map_it != pokemons_ptr->end()) {
                return map_it->second;
            }
            throw IteratorException("Iterator Exception: Invalid iterator");
        }
        
        Pokemon *operator->() const {
            if (pokemons_ptr && map_it != pokemons_ptr->end()) {
                return &(map_it->second);
            }
            throw IteratorException("Iterator Exception: Invalid iterator");
        }
        
        friend class Pokedex;
    };

    iterator begin() {
        return iterator(pokemons.begin(), &pokemons);
    }

    iterator end() {
        return iterator(pokemons.end(), &pokemons);
    }
};