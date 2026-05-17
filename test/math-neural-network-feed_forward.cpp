#include <rain.hpp>

using namespace Rain;
using namespace Algorithm;
using namespace Data;
using namespace Math;
using namespace Neural;
using namespace Multithreading;
using namespace Error;
using namespace std;

using LL = long long;
using LD = long double;
using CF = Clamped<LD>;

#define RF(x, from, to)                                    \
	for (                                                    \
		LL x(from), _to(to), _delta{x < _to ? 1LL : -1LL};     \
		x != _to;                                              \
		x += _delta)
size_t constexpr C_CLASS{10}, C_EPOCH{2};
CF constexpr STEP_SIZE{1e-1};

template<typename Value>
char pixelToChar(Value pixel) {
	if (pixel < 0x80) {
		return pixel < 0x40 ? ' ' : '-';
	} else {
		return pixel < 0xc0 ? '=' : '#';
	}
}

template<typename Value>
void showImg(
	Tensor<Value, 2> const &x,
	ostream &stream = cout) {
	x.template applyOver<1>([&](Tensor<Value, 1> const &row) {
		row.template applyOver<0>([&](Value const &pixel) {
			auto pixelChar{pixelToChar(pixel)};
			stream << pixelChar << pixelChar;
		});
		stream << '\n';
	});
}

int main(int, char const *const *const) {
	size_t C_THREAD{clamp(
		(size_t)thread::hardware_concurrency(), 1_zu, 8_zu)},
		BATCH_SIZE{C_THREAD * 32_zu},
		MINI_BATCH_SIZE{BATCH_SIZE / C_THREAD};
	cout << "Threads: " << C_THREAD << '.' << endl;

	filesystem::path assetPath{
		std::string(__FILE__) + ".asset/"};
	assetPath.make_preferred();
	cout << "Asset path: " << assetPath << '.' << endl;

	// random_device rd;
	mt19937 gen(0);
	normal_distribution<LD> dist(0.0L, 0.1L);

	Tensor<uint8_t, 3> trainX, testX;
	Tensor<uint8_t, 1> trainY, testY;
	{
		ifstream fStream(assetPath / "mnist.hfm", ios::binary);
		HuffmanStreamBuf decoderBuf(*fStream.rdbuf());
		istream decoderStream(&decoderBuf);
		Deserializer deserializer(decoderStream);
		deserializer >> trainX >> trainY >> testX >> testY;
	}

	// For this test, slice the trainset to 8K and the
	// testset to 1K.
	trainX.slice({{{0, 20480}, {}, {}}});
	trainY.slice({{{0, 20480}}});
	testX.slice({{{0, 1024}, {}, {}}});
	testY.slice({{{0, 1024}}});

	auto trainXDbl{trainX.asReshape<2>({trainX.size()[0], 0})
			.asRetype<CF>()},
		testXDbl{testX.asReshape<2>({testX.size()[0], 0})
				.asRetype<CF>()};
	Tensor<CF, 2> trainYOneHot({trainY.size()[0], C_CLASS}),
		testYOneHot({testY.size()[0], C_CLASS});
	trainYOneHot.applyOver<1>(
		[&](Tensor<CF, 1> &left, uint8_t const &right) {
			left[right] = 1;
		},
		trainY);
	testYOneHot.applyOver<1>(
		[&](Tensor<CF, 1> &left, uint8_t const &right) {
			left[right] = 1;
		},
		testY);
	trainXDbl /= 256;
	testXDbl /= 256;

	Network::FeedForward<CF> network(
		{make_shared<Activation::Linear<CF>>(
			 Tensor<CF, 2>({784, 64}, gen, dist),
			 Tensor<CF, 1>({64}, gen, dist)),
			make_shared<Activation::Relu<CF>>(),
			make_shared<Activation::Linear<CF>>(
				Tensor<CF, 2>({64, 10}, gen, dist),
				Tensor<CF, 1>({10}, gen, dist)),
			make_shared<Activation::Softmax<CF>>()});
	Loss::CrossEntropy<CF> L;

	vector<CF> lossV, scoreV;
	{
		ThreadPool tp(C_THREAD);

		CF stepSizeScaler{1.0L};
		RF(k, 0, C_EPOCH) {
			size_t cBatchTrain{trainXDbl.size()[0] / BATCH_SIZE};
			// size_t cBatchTrain{8};
			{
				vector<vector<Tensor<CF, 2>>> activationV(C_THREAD),
					activationGradientV(C_THREAD);
				mutex coutMtx;

				RF(i, 0, cBatchTrain) {
					atomic_size_t jOuter{};
					RF(j, 0, C_THREAD) {
						tp.queueTask([&]() {
							size_t jInner{jOuter++};
							auto X{trainXDbl.asSlice(
								{{{(i * C_THREAD + jInner) *
											MINI_BATCH_SIZE,
										(i * C_THREAD + jInner + 1) *
											MINI_BATCH_SIZE},
									{}}})},
								Y{trainYOneHot.asSlice(
									{{{(i * C_THREAD + jInner) *
												MINI_BATCH_SIZE,
											(i * C_THREAD + jInner + 1) *
												MINI_BATCH_SIZE},
										{}}})};
							activationV[jInner] = network.asApply(X);
							// cout << activation.back() << endl;
							activationGradientV[jInner] =
								network.getActivationGradient(
									L, Y, activationV[jInner]);

							auto loss{
								L.asApply(Y, activationV[jInner].back())};
							lock_guard coutLck(coutMtx);
							cout << "Mini-batch " << i * C_THREAD + jInner
									 << " / " << cBatchTrain * C_THREAD
									 << ": loss = " << loss << ".    \r"
									 << flush;
						});
					}
					tp.blockForTasks();
					RF(j, 0, C_THREAD) {
						network.stepWithActivationGradient(
							activationV[j],
							activationGradientV[j],
							STEP_SIZE * stepSizeScaler);
					}
				}
			}

			atomic<float> lossSum{};
			Tensor<size_t, 1> score({testXDbl.size()[0]});
			size_t cBatchTest{testXDbl.size()[0] / BATCH_SIZE};
			{
				mutex lossSumMtx;

				RF(i, 0, cBatchTest) {
					atomic_size_t jOuter{};
					RF(j, 0, C_THREAD) {
						tp.queueTask([&]() {
							size_t jInner{jOuter++};
							auto X{testXDbl.asSlice(
								{{{(i * C_THREAD + jInner) *
											MINI_BATCH_SIZE,
										(i * C_THREAD + jInner + 1) *
											MINI_BATCH_SIZE},
									{}}})},
								Y{testYOneHot.asSlice(
									{{{(i * C_THREAD + jInner) *
												MINI_BATCH_SIZE,
											(i * C_THREAD + jInner + 1) *
												MINI_BATCH_SIZE},
										{}}})};
							auto activationBack{
								network.asApply(X).back()};
							auto loss{L.asApply(Y, activationBack)};
							score
								.asSlice(
									{{{(i * C_THREAD + jInner) * MINI_BATCH_SIZE, (i * C_THREAD + jInner + 1) * MINI_BATCH_SIZE}}})
								.applyOver<0>(
									[](
										size_t &left,
										uint8_t const &r1,
										Tensor<CF, 1> const &r2) {
										left = r1 == r2.argMax();
									},
									testY.asSlice(
										{{{(i * C_THREAD + jInner) * MINI_BATCH_SIZE, (i * C_THREAD + jInner + 1) * MINI_BATCH_SIZE}}}),
									activationBack);

							lock_guard lossSumLck(lossSumMtx);
							lossSum += (float)loss;
						});
					}
					// Can also place outside i loop.
					tp.blockForTasks();
				}
			}

			// stepSizeScaler *= 0.999;
			// cout << score.asSlice({{{0, 256}}}) << endl;
			lossV.push_back(lossSum / cBatchTest / C_THREAD);
			scoreV.push_back(
				(float)score.sum() / cBatchTest / BATCH_SIZE);
			cout << "Epoch " << k << ": loss = " << lossV.back()
					 << ", score = " << scoreV.back() << '.' << endl;
		}
	}

	// This should work regardless of platform randomization.
	releaseAssert(scoreV.back() > 0.9);

	{
		stringstream ss;
		{
			Serializer serializer(ss);
			serializer << network << lossV << scoreV;
		}
		ofstream serializerStream(
			assetPath / ".network.tmp.hfm", ios::binary);
		HuffmanStreamBuf encoderBuf(
			*serializerStream.rdbuf(), ss.str());
		ostream encoder(&encoderBuf);
		encoder << ss.rdbuf();
		encoder.flush();
	}

	// Remove the unnecessary model checkpoint for this test.
	filesystem::remove(assetPath / ".network.tmp.hfm");

	return 0;
}
