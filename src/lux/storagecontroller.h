#ifndef LUX_STORAGECONTROLLER_H
#define LUX_STORAGECONTROLLER_H

#include "uint256.h"
#include "net.h"
#include "protocol.h"
#include "storageheap.h"
#include "storageorder.h"
#include "proposalsagent.h"
#include "decryptionkeys.h"

#include <boost/thread.hpp>
#include <openssl/rsa.h>

class StorageController;

extern std::unique_ptr<StorageController> storageController;

static const size_t STORAGE_MIN_RATE = 1;
static const int DEFAULT_STORAGE_MAX_BLOCK_GAP = 100;

static const uint64_t DEFAULT_STORAGE_SIZE = 100ull * 1024 * 1024 * 1024; // 100 Gb
static const unsigned short DEFAULT_DFS_PORT = 1507;

class StorageController
{
protected:
    boost::mutex mutex;
    boost::thread background;
    std::map<uint256, boost::filesystem::path> mapLocalFiles;
    std::map<uint256, StorageOrder> mapAnnouncements;
    std::map<uint256, StorageHandshake> mapReceivedHandshakes;

    bool StartHandshake(const StorageProposal &proposal, const DecryptionKeys &keys);
    DecryptionKeys GenerateKeys(RSA **rsa);
    RSA* CreatePublicRSA(const std::string &key);
    std::shared_ptr<AllocatedFile> CreateReplica(const boost::filesystem::path &filename,
                                                 const StorageOrder &order,
                                                 const DecryptionKeys &keys,
                                                 RSA *rsa);
    bool SendReplica(const StorageOrder &order, const uint256 merkleRootHash, std::shared_ptr<AllocatedFile> pAllocatedFile, CNode* pNode);
    bool CheckReceivedReplica(const uint256 &orderHash, const uint256 &receivedMerkleRootHash, const boost::filesystem::path &replica);
    bool DecryptReplica(std::shared_ptr<AllocatedFile> pAllocatedFile, const uint64_t fileSize, const boost::filesystem::path &decryptedFile);
    void BackgroundJob();

public:
    CAmount rate;
    int maxblocksgap;
    CAddress address;
    StorageHeap storageHeap;
    StorageHeap tempStorageHeap;
    ProposalsAgent proposalsAgent;

    StorageController() : background(boost::bind(&StorageController::BackgroundJob, this)),
                                     rate(STORAGE_MIN_RATE),
                                     maxblocksgap(DEFAULT_STORAGE_MAX_BLOCK_GAP) {}
    // Init functions
    void InitStorages(const boost::filesystem::path &dataDir, const boost::filesystem::path &tempDataDir);
    // Get functions
    std::map<uint256, StorageOrder> GetAnnouncements();
    const StorageOrder *GetAnnounce(const uint256 &hash);
    // Common functions
    void AnnounceOrder(const StorageOrder &order);
    void AnnounceOrder(const StorageOrder &order, const boost::filesystem::path &path);
    bool CancelOrder(const uint256 &orderHash);
    std::vector<StorageProposal> SortProposals(const StorageOrder &order);
    bool AcceptProposal(const StorageProposal &proposal);
    void ClearOldAnnouncments(std::time_t timestamp);
    void ProcessStorageMessage(CNode* pfrom, const std::string& strCommand, CDataStream& vRecv, bool& isStorageCommand);
    void DecryptReplica(const uint256 &orderHash, const boost::filesystem::path &decryptedFile);
    // Proposals Agent functions
    //void ListenProposal(const uint256 &orderHash);
    //void StopListenProposal(const uint256 &orderHash);
    //bool isListen(const uint256 &proposalHash);
    //std::vector<StorageProposal> GetProposals(const uint256 &orderHash);
    // Temp functions
    bool FindReplicaKeepers(const StorageOrder &order, const int countReplica);
};

#endif //LUX_STORAGECONTROLLER_H
