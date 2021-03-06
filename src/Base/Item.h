/**
   @author Shin'ichiro Nakaoka
*/

#ifndef CNOID_BASE_ITEM_H
#define CNOID_BASE_ITEM_H

#include "PutPropertyFunction.h"
#include <cnoid/Referenced>
#include <cnoid/Signal>
#include <cnoid/NullOut>
#include <ctime>
#include <bitset>
#include <string>
#include <list>
#include "exportdecl.h"

namespace cnoid {

class Item;
typedef ref_ptr<Item> ItemPtr;
    
class RootItem;
class ItemTreeArchiver;
class ExtensionManager;
class PutPropertyFunction;
class Archive;

/**
   @if not jp
   @endif

   @if jp
   フレームワーク上で共有されるオブジェクトを表すクラス。   
   モデル・ビュー・コントローラフレームワークにおけるモデル部分の核となる。   
   @endif
*/
class CNOID_EXPORT Item : public Referenced
{
public:

    enum Attribute {
        SUB_ITEM,
        TEMPORAL,
        LOAD_ONLY,
        NUM_ATTRIBUTES
    };

    Item();
    Item(const Item& item);
    virtual ~Item(); // The destructor should not be called in usual ways

    const std::string& name() const { return name_; }
    virtual void setName(const std::string& name);

    bool hasAttribute(Attribute attribute) const { return attributes[attribute]; }

    Item* childItem() const { return firstChild_; }
    Item* prevItem() const { return prevItem_; }
    Item* nextItem() const { return nextItem_; }
    Item* parentItem() const { return parent_; }

    bool addChildItem(ItemPtr item, bool isManualOperation = false);
    bool addSubItem(ItemPtr item);
    bool isSubItem() const;
    //int subItemIndex() const;
    //Item* subItem(int subItemIndex);
    void detachFromParentItem();
    void emitSigDetachedFromRootForSubTree();
    bool insertChildItem(ItemPtr item, ItemPtr nextItem, bool isManualOperation = false);
    bool insertSubItem(ItemPtr item, ItemPtr nextItem);

    bool isTemporal() const;
    void setTemporal(bool on = true);

    RootItem* findRootItem() const;
    Item* findItem(const std::string& path) const;

    template<class ItemType>
        inline ItemType* findItem(const std::string& path) const {
        return dynamic_cast<ItemType*>(findItem(path));
    }

    Item* findSubItem(const std::string& path) const;

    template<class ItemType>
        inline ItemType* findSubItem(const std::string& path) const {
        return dynamic_cast<ItemType*>(findSubItem(path));
    }
    
    /*
      The function 'template <class ItemType> ItemList<ItemType> getSubItems() const'
      has been removed. Please use ItemList::extractChildItems(Item* item) instead of it.
    */

    Item* headItem() const;

    template <class ItemType> ItemType* findOwnerItem(bool includeSelf = false) {
        Item* parentItem__ = includeSelf ? this : parentItem();
        while(parentItem__){
            ItemType* ownerItem = dynamic_cast<ItemType*>(parentItem__);
            if(ownerItem){
                return ownerItem;
            }
            parentItem__ = parentItem__->parentItem();
        }
        return 0;
    }

    void traverse(boost::function<void(Item*)> function);

    ItemPtr duplicate() const;
    ItemPtr duplicateAll() const;

    void assign(Item* srcItem);

    bool load(const std::string& filename, const std::string& format = std::string());
    bool load(const std::string& filename, Item* parent, const std::string& format = std::string());
    bool save(const std::string& filename, const std::string& format = std::string());
    bool overwrite(bool forceOverwrite = false, const std::string& format = std::string());

    const std::string& filePath() const { return filePath_; }
    const std::string& fileFormat() const { return fileFormat_; }

#ifdef CNOID_BACKWARD_COMPATIBILITY
    const std::string& lastAccessedFilePath() const { return filePath_; }
    const std::string& lastAccessedFileFormatId() const { return fileFormat_; }
#endif

    std::time_t fileModificationTime() const { return fileModificationTime_; }
    bool isConsistentWithFile() const { return isConsistentWithFile_; }

    void clearFileInformation();

    void suggestFileUpdate() { isConsistentWithFile_ = false; }

    void putProperties(PutPropertyFunction& putProperty);

    virtual void notifyUpdate();

    SignalProxy<void(const std::string& oldName)> sigNameChanged() {
        return sigNameChanged_;
    }

    SignalProxy<void()> sigUpdated() {
        return sigUpdated_;
    }

    /**
       This signal is emitted when the position of this item in the item tree is changed.
       Being added to the tree and being removed from the tree are also the events
       to emit this signal.
       This signal is also emitted for descendent items when the position of an ancestor item
       is changed.
       This signal is emitted before RootItem::sigTreeChanged();
    */
    SignalProxy<void()> sigPositionChanged() {
        return sigPositionChanged_;
    }

    /**
       @note obsolete.
    */
    SignalProxy<void()> sigDetachedFromRoot() {
        return sigDetachedFromRoot_;
    }
    /**
       @note Please use this instead of sigDetachedFromRoot()
    */
    SignalProxy<void()> sigDisconnectedFromRoot() {
        return sigDetachedFromRoot_;
    }

    SignalProxy<void()> sigSubTreeChanged() {
        return sigSubTreeChanged_;
    }

    virtual bool store(Archive& archive);
    virtual bool restore(const Archive& archive);

    Referenced* customData(int id);
    const Referenced* customData(int id) const;
    void setCustomData(int id, ReferencedPtr data);
    void clearCustomData(int id);

    static SignalProxy<void(const char* type_info_name)> sigClassUnregistered() {
        return sigClassUnregistered_;
    }

protected:

    virtual void onConnectedToRoot();
    virtual void onDisconnectedFromRoot();
    virtual void onPositionChanged();
    virtual bool onChildItemAboutToBeAdded(Item* childItem, bool isManualOperation);
        
    virtual ItemPtr doDuplicate() const;
    virtual void doAssign(Item* srcItem);
    virtual void doPutProperties(PutPropertyFunction& putProperty);

    void setAttribute(Attribute attribute) { attributes.set(attribute); }
    inline void unsetAttribute(Attribute attribute) { attributes.reset(attribute); }

private:

    Item* parent_;
    Item* firstChild_;
    Item* lastChild_;
    Item* prevItem_;
    Item* nextItem_;

    int numChildren_;

    std::string name_;

    std::bitset<NUM_ATTRIBUTES> attributes;

    std::vector<int> extraStates;
    std::vector<ReferencedPtr> extraData;

    Signal<void(const std::string& oldName)> sigNameChanged_;
    Signal<void()> sigDetachedFromRoot_;
    Signal<void()> sigUpdated_;
    Signal<void()> sigPositionChanged_;
    Signal<void()> sigSubTreeChanged_;

    static Signal<void(const char* type_info_name)> sigClassUnregistered_;

    // for file overwriting management, mainly accessed by ItemManagerImpl
    bool isConsistentWithFile_;
    std::string filePath_;
    std::string fileFormat_;
    std::time_t fileModificationTime_;

    // disable the assignment operator
    Item& operator=(const Item& rhs);

    void init();
    bool doInsertChildItem(Item* item, Item* nextItem, bool isManualOperation);
    void callSlotsOnPositionChanged();
    void callFuncOnConnectedToRoot();
    void addToItemsToEmitSigSubTreeChanged();
    void addToItemsToEmitSigSubTreeChangedSub(std::list<Item*>::iterator& pos);
    void emitSigSubTreeChanged();

    void detachFromParentItemSub(bool isMoving);
    void traverse(Item* item, const boost::function<void(Item*)>& function);
    ItemPtr duplicateAllSub(ItemPtr duplicated) const;
        
    void updateFileInformation(const std::string& filename, const std::string& format);
        
    friend class RootItem;
    friend class ItemTreeArchiver;
    friend class ItemManagerImpl;
};

#ifndef CNOID_BASE_MVOUT_DECLARED
#define CNOID_BASE_MVOUT_DECLARED
CNOID_EXPORT std::ostream& mvout(bool doFlush = false);
#endif

}

#endif
