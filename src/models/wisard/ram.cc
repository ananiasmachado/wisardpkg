
class RAM{
public:
  RAM(){}
  RAM(nl::json c){
    ignoreZero = c["ignoreZero"];
    base=c["base"];
    addresses = c["addresses"].get<std::vector<int>>();
    checkLimitAddressSize(addresses.size(), base);
    setData(c["data"]);
    // nl::json pos = c["positions"];
    // for(nl::json::iterator it = pos.begin(); it != pos.end(); ++it){
    //   unsigned long long p = stoull(it.key());
    //   positions[p] = it.value();
    // }
  }
  RAM(const int addressSize, const int entrySize, const bool ignoreZero=false, int base=2): ignoreZero(ignoreZero), base(base){
    checkLimitAddressSize(addressSize, base);
    addresses = std::vector<int>(addressSize);
    generateRandomAddresses(entrySize);
  }
  RAM(const std::vector<int> indexes, const bool ignoreZero=false, int base=2): addresses(indexes), ignoreZero(ignoreZero), base(base){
    checkLimitAddressSize(indexes.size(), base);
  }

  int getVote(const std::vector<int>& image){
    unsigned long long index = getIndex(image);
    if(ignoreZero && index == 0)
      return 0;
    auto it = positions.find(index);
    if(it == positions.end()){
      return 0;
    }
    else{
      return it->second;
    }
  }

  void train(const std::vector<int>& image){
    unsigned long long index = getIndex(image);
    auto it = positions.find(index);
    if(it == positions.end()){
      positions.insert(it,std::pair<unsigned long long,int>(index, 1));
    }
    else{
      it->second++;
    }
  }

  void untrain(const std::vector<int>& image){
      unsigned long long index = getIndex(image);
      auto it = positions.find(index);
      if(it != positions.end()){
        it->second--;
      }
  }

  std::vector<std::vector<int>> getMentalImage() {
    std::vector<std::vector<int>> mentalPiece(addresses.size());
    for(unsigned int i=0; i<mentalPiece.size(); i++){
      mentalPiece[i].resize(2);
      mentalPiece[i][0] = addresses[i];
      mentalPiece[i][1] = 0;
    }

    for(auto j=positions.begin(); j!=positions.end(); ++j){
      if(j->first == 0) continue;
      const std::vector<int> address = convertToBase(j->first);
      for(unsigned int i=0; i<mentalPiece.size(); i++){
        if(mentalPiece[i].size() == 0){
          mentalPiece[i].resize(2);
          mentalPiece[i][0] = addresses[i];
          mentalPiece[i][1] = 0;
        }
        if(address[i] > 0){
          mentalPiece[i][1] += j->second;
        }
      }
    }
    return mentalPiece;
  }

  nl::json getConfig(){
    nl::json config = {
      {"ignoreZero", ignoreZero},
      {"base", base}
    };
    return config;
  }

  void setData(std::string data){
    std::string decodedData = Base64::decode(data);
    const int base1 = sizeof(unsigned long long);
    const int base2 = sizeof(int);
    const int blockSize = (sizeof(unsigned long long)+sizeof(int));

    for(unsigned long i=0; i<decodedData.size(); i+=blockSize){
      unsigned long long address = 0;
      int value = 0;

      for(int j=0; j<base1; j++){
        unsigned long long temp = decodedData[i+j];
        address |= (temp << (8*j));
      }
      for(int k=0; k<base2; k++){
        int temp = decodedData[i+k+base1];
        value |= (temp << (8*k));
      }
      positions[address]=value;
    }
  }

  std::string getData(){
    int blockSize = (sizeof(unsigned long long)+sizeof(int));
    std::string data(positions.size()*blockSize,0);
    int k=0;
    for(auto j=positions.begin(); j!=positions.end(); ++j){
      for(unsigned int i=0; i<sizeof(unsigned long long); i++){
        data[k++] = (j->first >> (8*i)) & 0xff;
      }
      for(unsigned int i=0; i<sizeof(int); i++){
        data[k++] = (j->second >> (8*i)) & 0xff;
      }
    }
    return Base64::encode(data);
  }

  void setMapping(std::vector<std::vector<int>>& mapping, int i){
    int size = addresses.size();
    mapping[i].resize(size);
    for(int j=0; j<size; j++) {
      mapping[i][j] = addresses[j];
    }
  }

  // nl::json getJSON(bool all=true){
  //   nl::json config = {
  //     {"addresses", addresses},
  //     {"positions", nullptr}
  //   };
  //   if(all){
  //     nl::json pos;
  //     for(auto j=positions.begin(); j!=positions.end(); ++j){
  //       pos[std::to_string(j->first)] = j->second;
  //     }
  //     config["positions"] = pos;
  //   }
  //   return config;
  // }

  int getAddressSize(){
    return addresses.size();
  }

  ~RAM(){
    addresses.clear();
    positions.clear();
  }

protected:
  unsigned long long getIndex(const std::vector<int>& image) const{
    unsigned long long index = 0;
    unsigned long long p = 1;
    for(unsigned int i=0; i<addresses.size(); i++){
      int bin = image[addresses[i]];
      checkPos(bin);
      index += bin*p;
      p *= base;
    }
    return index;
  }


private:
  std::vector<int> addresses;
  std::unordered_map<unsigned long long,int> positions;
  bool ignoreZero;
  int base;

  const std::vector<int> convertToBase(const int number) const{
    std::vector<int> numberConverted(addresses.size());
    int baseNumber = number;
    for(unsigned int i=0; i<numberConverted.size(); i++){
      numberConverted[i] = baseNumber % base;
      baseNumber /= base;
    }
    return numberConverted;
  }

  void checkLimitAddressSize(int addressSize, int basein){
    const unsigned long long limit = -1;
    if((basein == 2 && addressSize > 64) ||
       (basein != 2 && (unsigned long long)ipow(basein,addressSize) > limit)){
      throw Exception("The base power to addressSize passed the limit of 2^64!");
    }
  }

  void checkPos(const int code) const{
    if(code >= base){
      throw Exception("The input data has a value bigger than base of addresing!");
    }
  }

  void generateRandomAddresses(int entrySize){
    for(unsigned int i=0; i<addresses.size(); i++){
      addresses[i] = randint(0, entrySize-1);
    }
  }
};
