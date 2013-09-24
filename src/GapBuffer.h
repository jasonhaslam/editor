#ifndef GAP_BUFFER_H
#define GAP_BUFFER_H

#include <cassert>
#include <QTextStream>

template <typename T>
class GapBuffer;

template <typename T>
inline QTextStream &operator<<(QTextStream &rhs, const GapBuffer<T> &lhs)
{
  for (int i = 0; i < lhs.mGapPos; ++i)
    rhs << lhs.mData[i];

  rhs << "[" << QString(lhs.mGapLen, ' ') << "]";

  T *tail = lhs.mData + lhs.mGapPos + lhs.mGapLen;
  int len = lhs.mLength - lhs.mGapPos - lhs.mGapLen;
  for (int i = 0; i < len; ++i)
    rhs << tail[i];

  return rhs;
}

template <typename T>
class GapBuffer
{
public:
  GapBuffer(int capacity = 20);
  ~GapBuffer();

  int length() const { return mLength - mGapLen; }
  int gapPosition() const { return mGapPos; }

  void insert(int pos, T value);
  void insert(int pos, const T *data, int len);

  void remove(int pos, int len = 1);

  T at(int pos) const;
  T &operator[](int pos);

  T *data(int pos, int len);
  const T *constData(int pos) const;

private:
  void moveGap(int pos);
  void ensureCapacity(int size);

  T *mData;
  int mLength;
  int mGapPos;
  int mGapLen;

  friend QTextStream &operator<<<T>(QTextStream &rhs, const GapBuffer<T> &lhs);
};

template <typename T>
GapBuffer<T>::GapBuffer(int cap)
  : mData(new T[cap]), mLength(cap), mGapPos(0), mGapLen(cap)
{}

template <typename T>
GapBuffer<T>::~GapBuffer()
{
  delete [] mData;
}

template <typename T>
void GapBuffer<T>::insert(int pos, T value)
{
  if (pos > length())
    return;

  ensureCapacity(1);
  moveGap(pos);

  mData[pos] = value;
  ++mGapPos;
  --mGapLen;
}

template <typename T>
void GapBuffer<T>::insert(int pos, const T *data, int len)
{
  if (pos > length())
    return;

  ensureCapacity(len);
  moveGap(pos);

  memcpy(mData + pos, data, sizeof(T) * len);
  mGapPos += len;
  mGapLen -= len;
}

template <typename T>
void GapBuffer<T>::remove(int pos, int len)
{
  if (len == 0 || pos + len > length())
    return;

  moveGap(pos);
  mGapLen += len;
}

template <typename T>
T GapBuffer<T>::at(int pos) const
{
  if (pos >= length())
    return 0;

  return mData[pos < mGapPos ? pos : pos + mGapLen];
}

template <typename T>
T &GapBuffer<T>::operator[](int pos)
{
  assert(pos < length());
  return mData[pos < mGapPos ? pos : pos + mGapLen];
}

template <typename T>
T *GapBuffer<T>::data(int pos, int len)
{
  moveGap(pos + len);
  mData[mGapPos] = T();
  return mData + pos;
}

template <typename T>
const T *GapBuffer<T>::constData(int pos) const
{
  return mData + (pos < mGapPos ? pos : pos + mGapLen);
}

template <typename T>
void GapBuffer<T>::moveGap(int pos)
{
  if (pos == mGapPos)
    return;

  if (pos < mGapPos) {
    memmove(mData + pos + mGapLen,
            mData + pos,
            sizeof(T) * (mGapPos - pos));
  } else {
    memmove(mData + mGapPos,
            mData + mGapPos + mGapLen,
            sizeof(T) * (pos - mGapPos));
  }

  mGapPos = pos;
}

template <typename T>
void GapBuffer<T>::ensureCapacity(int size)
{
  if (size < mGapLen)
    return;

  // Move gap to end.
  moveGap(length());

  int len = mLength;
  while (mGapLen <= size) {
    int inc = (mLength < 4084) ? mLength + 12 : 4096;
    mLength += inc;
    mGapLen += inc;
  }

  T *data = new T[mLength];
  memcpy(data, mData, sizeof(T) * len);
  delete [] mData;
  mData = data;
}

#endif
